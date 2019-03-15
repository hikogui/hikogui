//
//  Window.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Window.hpp"

#include "Device.hpp"
#include "WindowView.hpp"
#include "config.hpp"

#include "Logging.hpp"

#include <boost/numeric/conversion/cast.hpp>

namespace TTauri {

using namespace std;

#pragma mark "Public"
// Public methods need to lock the instance.

Window::Window(Instance *instance, std::shared_ptr<Delegate> delegate, const std::string &title, vk::SurfaceKHR surface) :
    state(State::NO_DEVICE),
    instance(instance),
    delegate(delegate),
    title(title),
    intrinsic(surface)
{
    view = std::make_shared<WindowView>(this);
    backingPipeline = std::make_shared<BackingPipeline>(this);
}

Window::~Window() {}

void Window::initialize()
{
    delegate->initialize(this);
}

void Window::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync)
{
    if (stateMutex.try_lock()) {
        if (state == State::READY_TO_DRAW) {
            if (!render(blockOnVSync)) {
                LOG_INFO("Swapchain out of date.");
                state = State::SWAPCHAIN_OUT_OF_DATE;
            }
        }
        stateMutex.unlock();
    }
}

void Window::maintenance()
{
    std::scoped_lock lock(stateMutex);

    if (state == State::SWAPCHAIN_OUT_OF_DATE || state == State::MINIMIZED) {
        state = State::SWAPCHAIN_OUT_OF_DATE;

        auto onScreen = rebuildForSwapchainChange();

        state = onScreen ? State::READY_TO_DRAW : State::MINIMIZED;
    }
}

void Window::buildForDeviceChange()
{
    std::scoped_lock lock(stateMutex);

    if (state == State::LINKED_TO_DEVICE) {
        auto swapchainAndOnScreen = buildSwapchain();
        swapchain = swapchainAndOnScreen.first;
        auto onScreen = swapchainAndOnScreen.second;

        buildRenderPasses();
        buildFramebuffers();
        buildSemaphores();
        backingPipeline->buildForDeviceChange(firstRenderPass, swapchainCreateInfo.imageExtent, swapchainFramebuffers.size());

        state = onScreen ? State::READY_TO_DRAW : State::MINIMIZED;

    } else {
        BOOST_THROW_EXCEPTION(Window::StateError());
    }
}

void Window::teardownForDeviceChange()
{
    std::scoped_lock lock(stateMutex);

    if (state == State::READY_TO_DRAW || state == State::SWAPCHAIN_OUT_OF_DATE || state == State::MINIMIZED) {
        waitIdle();
        backingPipeline->teardownForDeviceChange();
        teardownSemaphores();
        teardownFramebuffers();
        teardownRenderPasses();
        device->intrinsic.destroy(swapchain);
        state = State::LINKED_TO_DEVICE;
    } else {
        BOOST_THROW_EXCEPTION(Window::StateError());
    }
}


bool Window::rebuildForSwapchainChange()
{
    if (!isOnScreen()) {
        // Early exit when window is minimized.
        return false;
    }

    waitIdle();

    backingPipeline->teardownForSwapchainChange();
    teardownFramebuffers();

    auto swapChainAndOnScreen = buildSwapchain(swapchain);
    swapchain = swapChainAndOnScreen.first;
    auto onScreen = swapChainAndOnScreen.second;

    buildFramebuffers();
    backingPipeline->buildForSwapchainChange(firstRenderPass, swapchainCreateInfo.imageExtent, swapchainFramebuffers.size());

    return onScreen;
}

void Window::waitIdle()
{
    device->intrinsic.waitForFences(1, &renderFinishedFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

void Window::setDevice(Device *device)
{
    if (device) {
        {
            std::scoped_lock lock(stateMutex);

            if (state == State::NO_DEVICE) {
                this->device = device;
                state = State::LINKED_TO_DEVICE;

            } else {
                BOOST_THROW_EXCEPTION(Window::StateError());
            }
        }

        buildForDeviceChange();

    } else {
        teardownForDeviceChange();

        {
            std::scoped_lock lock(stateMutex);

            if (state == State::LINKED_TO_DEVICE) {
                this->device = nullptr;
                state = State::NO_DEVICE;

            } else {
                BOOST_THROW_EXCEPTION(Window::StateError());
            }
        }
    }
}

#pragma mark "Private"
// Private methods don't need to lock.

std::pair<uint32_t, vk::Extent2D> Window::getImageCountAndImageExtent()
{
    auto surfaceCapabilities = device->physicalIntrinsic.getSurfaceCapabilitiesKHR(intrinsic);

    uint32_t imageCount;
    if (surfaceCapabilities.maxImageCount) {
        imageCount = std::clamp(defaultNumberOfSwapchainImages, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

    } else {
        imageCount = std::max(defaultNumberOfSwapchainImages, surfaceCapabilities.minImageCount);
    }

    vk::Extent2D imageExtent;
    if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max() &&
        surfaceCapabilities.currentExtent.height == std::numeric_limits<uint32_t>::max()) {
        LOG_WARNING("getSurfaceCapabilitiesKHR() does not supply currentExtent");
        imageExtent.width =
            std::clamp(windowRectangle.extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        imageExtent.height =
            std::clamp(windowRectangle.extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

    } else {
        imageExtent = surfaceCapabilities.currentExtent;
    }

    return { imageCount, imageExtent };
}

bool Window::isOnScreen()
{
    auto imageCountAndImageExtent = getImageCountAndImageExtent();

    return imageCountAndImageExtent.second.width > 0 && imageCountAndImageExtent.second.height > 0;
}

std::pair<vk::SwapchainKHR, bool> Window::buildSwapchain(vk::SwapchainKHR oldSwapchain)
{
    // Figure out the best way of sharing data between the present and graphic queues.
    vk::SharingMode sharingMode;
    uint32_t sharingQueueFamilyCount;
    uint32_t sharingQueueFamilyIndices[2] = { device->graphicQueue->queueFamilyIndex, device->presentQueue->queueFamilyIndex };
    uint32_t *sharingQueueFamilyIndicesPtr;

    if (device->presentQueue->queueCapabilities.handlesGraphicsAndPresent()) {
        sharingMode = vk::SharingMode::eExclusive;
        sharingQueueFamilyCount = 0;
        sharingQueueFamilyIndicesPtr = nullptr;
    } else {
        sharingMode = vk::SharingMode::eConcurrent;
        sharingQueueFamilyCount = 2;
        sharingQueueFamilyIndicesPtr = sharingQueueFamilyIndices;
    }

retry:
    auto imageCountAndImageExtent = getImageCountAndImageExtent();
    auto imageCount = imageCountAndImageExtent.first;
    auto imageExtent = imageCountAndImageExtent.second;

    if (imageExtent.width == 0 || imageExtent.height == 0) {
        return { oldSwapchain, false };
    }

    swapchainCreateInfo = vk::SwapchainCreateInfoKHR(
        vk::SwapchainCreateFlagsKHR(),
        intrinsic,
        imageCount,
        device->bestSurfaceFormat.format,
        device->bestSurfaceFormat.colorSpace,
        imageExtent,
        1, // imageArrayLayers
        vk::ImageUsageFlagBits::eColorAttachment,
        sharingMode,
        sharingQueueFamilyCount,
        sharingQueueFamilyIndicesPtr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        device->bestSurfacePresentMode,
        VK_TRUE, // clipped
        oldSwapchain);

    vk::SwapchainKHR newSwapchain;
    vk::Result result = device->intrinsic.createSwapchainKHR(&swapchainCreateInfo, nullptr, &newSwapchain);
    // No matter what, the oldSwapchain has been retired after createSwapchainKHR().
    device->intrinsic.destroy(oldSwapchain);
    oldSwapchain = vk::SwapchainKHR();

    if (result != vk::Result::eSuccess) {
        LOG_WARNING("Could not create swapchain, retrying.");
        goto retry;
    }

    auto checkImageCountAndImageExtent = getImageCountAndImageExtent();
    auto checkImageExtent = checkImageCountAndImageExtent.second;
    if (imageExtent != checkImageExtent) {
        LOG_WARNING("Surface extent changed while creating swapchain, retrying.");
        // The newSwapchain was created succesfully, it is just of the wrong size so use it as the next oldSwapchain.
        oldSwapchain = newSwapchain;
        goto retry;
    }

    view->setRectangle({ 0.0, 0.0, 0.0 }, { swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height, 0.0 });

    LOG_INFO("Building swap chain");
    LOG_INFO(" - extent=%i x %i") % swapchainCreateInfo.imageExtent.width % swapchainCreateInfo.imageExtent.height;
    LOG_INFO(" - colorSpace=%s, format=%s") % vk::to_string(swapchainCreateInfo.imageColorSpace) % vk::to_string(swapchainCreateInfo.imageFormat);
    LOG_INFO(" - presentMode=%s, imageCount=%i") % vk::to_string(swapchainCreateInfo.presentMode) % swapchainCreateInfo.minImageCount;

    return { newSwapchain, true };
}

void Window::buildFramebuffers()
{
    swapchainImages = device->intrinsic.getSwapchainImagesKHR(swapchain);
    for (auto image : swapchainImages) {
        uint32_t baseMipLlevel = 0;
        uint32_t levelCount = 1;
        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = 1;
        auto imageSubresourceRange =
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, baseMipLlevel, levelCount, baseArrayLayer, layerCount);

        auto imageViewCreateInfo = vk::ImageViewCreateInfo(
            vk::ImageViewCreateFlags(),
            image,
            vk::ImageViewType::e2D,
            swapchainCreateInfo.imageFormat,
            vk::ComponentMapping(),
            imageSubresourceRange);

        auto imageView = device->intrinsic.createImageView(imageViewCreateInfo);
        swapchainImageViews.push_back(imageView);

        std::vector<vk::ImageView> attachments = { imageView };

        auto framebufferCreateInfo = vk::FramebufferCreateInfo(
            vk::FramebufferCreateFlags(),
            firstRenderPass,
            boost::numeric_cast<uint32_t>(attachments.size()),
            attachments.data(),
            swapchainCreateInfo.imageExtent.width,
            swapchainCreateInfo.imageExtent.height,
            1 // layers
        );

        LOG_INFO("createFramebuffer (%i, %i)") % swapchainCreateInfo.imageExtent.width % swapchainCreateInfo.imageExtent.height;

        auto framebuffer = device->intrinsic.createFramebuffer(framebufferCreateInfo);
        swapchainFramebuffers.push_back(framebuffer);
    }

    BOOST_ASSERT(swapchainImageViews.size() == swapchainImages.size());
    BOOST_ASSERT(swapchainFramebuffers.size() == swapchainImages.size());
}

void Window::teardownFramebuffers()
{
    for (auto frameBuffer : swapchainFramebuffers) {
        device->intrinsic.destroy(frameBuffer);
    }
    swapchainFramebuffers.clear();

    for (auto imageView : swapchainImageViews) {
        device->intrinsic.destroy(imageView);
    }
    swapchainImageViews.clear();
}

void Window::buildRenderPasses()
{
    std::vector<vk::AttachmentDescription> attachmentDescriptions = { {
        vk::AttachmentDescriptionFlags(),
        swapchainCreateInfo.imageFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
        vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
        vk::ImageLayout::eUndefined, // initialLayout
        vk::ImageLayout::ePresentSrcKHR // finalLayout
    } };

    std::vector<vk::AttachmentReference> inputAttachmentReferences = {};

    std::vector<vk::AttachmentReference> colorAttachmentReferences = { { 0, vk::ImageLayout::eColorAttachmentOptimal } };

    std::vector<vk::SubpassDescription> subpassDescriptions = { { vk::SubpassDescriptionFlags(),
                                                                  vk::PipelineBindPoint::eGraphics,
                                                                  boost::numeric_cast<uint32_t>(inputAttachmentReferences.size()),
                                                                  inputAttachmentReferences.data(),
                                                                  boost::numeric_cast<uint32_t>(colorAttachmentReferences.size()),
                                                                  colorAttachmentReferences.data() } };

    std::vector<vk::SubpassDependency> subpassDependency = { { VK_SUBPASS_EXTERNAL,
                                                               0,
                                                               vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                               vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                               vk::AccessFlags(),
                                                               vk::AccessFlagBits::eColorAttachmentRead |
                                                                   vk::AccessFlagBits::eColorAttachmentWrite } };

    vk::RenderPassCreateInfo renderPassCreateInfo = { vk::RenderPassCreateFlags(),   boost::numeric_cast<uint32_t>(attachmentDescriptions.size()),
                                                      attachmentDescriptions.data(), boost::numeric_cast<uint32_t>(subpassDescriptions.size()),
                                                      subpassDescriptions.data(),    boost::numeric_cast<uint32_t>(subpassDependency.size()),
                                                      subpassDependency.data() };

    firstRenderPass = device->intrinsic.createRenderPass(renderPassCreateInfo);
    attachmentDescriptions[0].loadOp = vk::AttachmentLoadOp::eClear;
    followUpRenderPass = device->intrinsic.createRenderPass(renderPassCreateInfo);
}

void Window::teardownRenderPasses()
{
    device->intrinsic.destroy(firstRenderPass);
    device->intrinsic.destroy(followUpRenderPass);
}

void Window::buildSemaphores()
{
    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
    imageAvailableSemaphore = device->intrinsic.createSemaphore(semaphoreCreateInfo, nullptr);

    // This fence is used to wait for the Window and its Pipelines to be idle.
    // It should therefor be signed at the start so that when no rendering has been
    // done it is still idle.
    auto fenceCreateInfo = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
    renderFinishedFence = device->intrinsic.createFence(fenceCreateInfo, nullptr);
}

void Window::teardownSemaphores()
{
    device->intrinsic.destroy(imageAvailableSemaphore);
    device->intrinsic.destroy(renderFinishedFence);
}

bool Window::render(bool blockOnVSync)
{
    uint32_t imageIndex;
    uint64_t timeout = blockOnVSync ? std::numeric_limits<uint64_t>::max() : 0;

    auto result = device->intrinsic.acquireNextImageKHR(swapchain, timeout, imageAvailableSemaphore, vk::Fence(), &imageIndex);
    switch (result) {
    case vk::Result::eSuccess: break;
    case vk::Result::eSuboptimalKHR: LOG_INFO("acquireNextImageKHR() eSuboptimalKHR"); return false;
    case vk::Result::eErrorOutOfDateKHR: LOG_INFO("acquireNextImageKHR() eErrorOutOfDateKHR"); return false;
    case vk::Result::eTimeout:
        // Don't render, we didn't receive an image.
        return true;
    default: BOOST_THROW_EXCEPTION(Window::SwapChainError());
    }

    vk::Semaphore renderFinishedSemaphores[] = { backingPipeline->render(imageIndex, imageAvailableSemaphore) };

    // Make a fence that should be signaled when all drawing is finished.
    device->intrinsic.waitIdle();
    // device->intrinsic.waitForFences(1, &renderFinishedFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    device->intrinsic.resetFences(1, &renderFinishedFence);
    device->graphicQueue->intrinsic.submit(0, nullptr, renderFinishedFence);

    auto presentInfo = vk::PresentInfoKHR(1, renderFinishedSemaphores, 1, &swapchain, &imageIndex);

    // Pass present info as a pointer to get the non-throw version.
    result = device->presentQueue->intrinsic.presentKHR(&presentInfo);
    switch (result) {
    case vk::Result::eSuccess: break;
    case vk::Result::eSuboptimalKHR: LOG_INFO("presentKHR() eSuboptimalKHR"); return false;
    case vk::Result::eErrorOutOfDateKHR: LOG_INFO("presentKHR() eErrorOutOfDateKHR"); return false;
    default: BOOST_THROW_EXCEPTION(Window::SwapChainError());
    }
    return true;
}

}
