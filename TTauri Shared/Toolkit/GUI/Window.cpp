//
//  Window.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include <boost/numeric/conversion/cast.hpp>

#include "Window.hpp"
#include "Device.hpp"
#include "TTauri/Toolkit/Logging.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

using namespace std;

#pragma mark "Public"
// Public methods need to lock the instance.

Window::Window(Instance *instance, vk::SurfaceKHR surface) :
    state(WindowState::NO_DEVICE), instance(instance), intrinsic(surface)
{
    backingPipeline = std::make_shared<BackingPipeline>(this);
}

Window::~Window()
{
}

void Window::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync)
{
    if (stateMutex.try_lock()) {
        if (state == WindowState::READY_TO_DRAW) {
            if (!render(blockOnVSync)) {
                LOG_INFO("Swapchain out of date.");
                state = WindowState::SWAPCHAIN_OUT_OF_DATE;
            }
        }
        stateMutex.unlock();
    }
}

void Window::waitIdle(void)
{
    LOG_INFO("waitIdle");
    device->intrinsic.waitForFences(1, &renderFinishedFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    LOG_INFO("/waitIdle");
}

void Window::maintenance(void)
{
    std::scoped_lock lock(stateMutex);

    if (state == WindowState::SWAPCHAIN_OUT_OF_DATE) {
        LOG_INFO("Rebuilding swapchain 1. %i") % static_cast<int>(state);

        teardownSwapchainAndPipeline();
         LOG_INFO("Rebuilding swapchain 2. %i") % static_cast<int>(state);
       buildSwapchainAndPipeline();
         LOG_INFO("Rebuilding swapchain 3. %i") % static_cast<int>(state);
   }
}

void Window::buildSwapchainAndPipeline(void)
{
    std::scoped_lock lock(stateMutex);

    if (state == WindowState::LINKED_TO_DEVICE) {
        buildSwapchain();
        buildRenderPasses();
        buildFramebuffers();
        buildPipelines();
        buildSemaphores();
        state = WindowState::READY_TO_DRAW;
    } else {
        BOOST_THROW_EXCEPTION(WindowStateError());
    }
}

void Window::teardownSwapchainAndPipeline(void)
{
    std::scoped_lock lock(stateMutex);

    if (state == WindowState::READY_TO_DRAW || state == WindowState::SWAPCHAIN_OUT_OF_DATE) {
        waitIdle();
        teardownSemaphores();
        teardownPipelines();
        teardownFramebuffers();
        teardownRenderPasses();
        teardownSwapchain();
        state = WindowState::LINKED_TO_DEVICE;
    } else {
        BOOST_THROW_EXCEPTION(WindowStateError());
    }
}

void Window::setDevice(Device *device) {
    if (device) {
        {
            std::scoped_lock lock(stateMutex);

            if (state == WindowState::NO_DEVICE) {
                this->device = device;
                state = WindowState::LINKED_TO_DEVICE;

            } else {
                BOOST_THROW_EXCEPTION(WindowStateError());
            }
        }

        buildSwapchainAndPipeline();

    } else {
        teardownSwapchainAndPipeline();

        {
            std::scoped_lock lock(stateMutex);

            if (state == WindowState::LINKED_TO_DEVICE) {
                this->device = nullptr;
                state = WindowState::NO_DEVICE;

            } else {
                BOOST_THROW_EXCEPTION(WindowStateError());
            }
        }
    }
}

#pragma mark "Private"
// Private methods don't need to lock.


void Window::buildSwapchain(void)
{
    // Figure out the best way of sharing data between the present and graphic queues.
    vk::SharingMode sharingMode;
    uint32_t sharingQueueFamilyCount;
    uint32_t sharingQueueFamilyIndices[2] = {
        device->graphicQueue->queueFamilyIndex,
        device->presentQueue->queueFamilyIndex
    };
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

    uint32_t imageArrayLayers = 1;
    vk::Bool32 clipped = VK_TRUE;

    vk::Extent2D previousImageExtent;
    for (uint64_t raceConditionRetry = 0; ; raceConditionRetry++) {
        auto surfaceCapabilities = device->physicalIntrinsic.getSurfaceCapabilitiesKHR(intrinsic);

        auto imageCount = std::clamp(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

        vk::Extent2D imageExtent = surfaceCapabilities.currentExtent;
        if (imageExtent.width == std::numeric_limits<uint32_t>::max() or imageExtent.height == std::numeric_limits<uint32_t>::max()) {
            imageExtent.width = std::clamp(windowRectangle.extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            imageExtent.height = std::clamp(windowRectangle.extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
        }

        if (raceConditionRetry > 0) {
            if (imageExtent == previousImageExtent) {
                break;
            } else {
                LOG_ERROR("Race condition during resize of window and creating swapchain.");
            }
        }

        swapchainCreateInfo = vk::SwapchainCreateInfoKHR(
            vk::SwapchainCreateFlagsKHR(),
            intrinsic,
            imageCount,
            device->bestSurfaceFormat.format,
            device->bestSurfaceFormat.colorSpace,
            imageExtent,
            imageArrayLayers,
            vk::ImageUsageFlags(),
            sharingMode,
            sharingQueueFamilyCount,
            sharingQueueFamilyIndicesPtr,
            vk::SurfaceTransformFlagBitsKHR::eIdentity,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            device->bestSurfacePresentMode,
            clipped
        );

        swapchain = device->intrinsic.createSwapchainKHR(swapchainCreateInfo);

        previousImageExtent = imageExtent;
    }

    LOG_INFO("Building swap chain");
    LOG_INFO(" - extent=%i x %i") % swapchainCreateInfo.imageExtent.width % swapchainCreateInfo.imageExtent.height;
    LOG_INFO(" - colorSpace=%s, format=%s") % vk::to_string(swapchainCreateInfo.imageColorSpace) % vk::to_string(swapchainCreateInfo.imageFormat);
    LOG_INFO(" - presentMode=%s, imageCount=%i") % vk::to_string(swapchainCreateInfo.presentMode) % swapchainCreateInfo.minImageCount;

}

void Window::teardownSwapchain(void)
{
    LOG_INFO("Teardown swapchain");

    device->intrinsic.destroy(swapchain);
}

void Window::buildFramebuffers(void)
{
    swapchainImages = device->intrinsic.getSwapchainImagesKHR(swapchain);
    for (auto image: swapchainImages) {
        uint32_t baseMipLlevel = 0;
        uint32_t levelCount = 1;
        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = 1;
        auto imageSubresourceRange = vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor,
            baseMipLlevel,
            levelCount,
            baseArrayLayer,
            layerCount
        );

        auto imageViewCreateInfo = vk::ImageViewCreateInfo(
            vk::ImageViewCreateFlags(),
            image,
            vk::ImageViewType::e2D,
            swapchainCreateInfo.imageFormat,
            vk::ComponentMapping(),
            imageSubresourceRange
        );

        auto imageView = device->intrinsic.createImageView(imageViewCreateInfo);
        swapchainImageViews.push_back(imageView);

        std::vector<vk::ImageView> attachments = {imageView};

        auto framebufferCreateInfo = vk::FramebufferCreateInfo(
            vk::FramebufferCreateFlags(),
            firstRenderPass,
            boost::numeric_cast<uint32_t>(attachments.size()), attachments.data(),
            swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height,
            1 // layers
        );

        LOG_INFO("createFramebuffer (%i, %i)") % swapchainCreateInfo.imageExtent.width % swapchainCreateInfo.imageExtent.height;

        auto framebuffer = device->intrinsic.createFramebuffer(framebufferCreateInfo);
        swapchainFramebuffers.push_back(framebuffer);
    }

    BOOST_ASSERT(swapchainImageViews.size() == swapchainImages.size());
    BOOST_ASSERT(swapchainFramebuffers.size() == swapchainImages.size());
}

void Window::teardownFramebuffers(void)
{
    for (auto frameBuffer: swapchainFramebuffers) {
        device->intrinsic.destroy(frameBuffer);
    }
    swapchainFramebuffers.clear();

    for (auto imageView: swapchainImageViews) {
        device->intrinsic.destroy(imageView);
    }
    swapchainImageViews.clear();
}

void Window::buildRenderPasses(void)
{
    std::vector<vk::AttachmentDescription> attachmentDescriptions = {{
        vk::AttachmentDescriptionFlags(),
        swapchainCreateInfo.imageFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
        vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
        vk::ImageLayout::eUndefined, // initialLayout
        vk::ImageLayout::ePresentSrcKHR // finalLayout
    }};

    std::vector<vk::AttachmentReference> inputAttachmentReferences = {
    };
    
    std::vector<vk::AttachmentReference> colorAttachmentReferences = {
        {0, vk::ImageLayout::eColorAttachmentOptimal}
    };

    std::vector<vk::SubpassDescription> subpassDescriptions = {{
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        boost::numeric_cast<uint32_t>(inputAttachmentReferences.size()), inputAttachmentReferences.data(),
        boost::numeric_cast<uint32_t>(colorAttachmentReferences.size()), colorAttachmentReferences.data()
    }};

    std::vector<vk::SubpassDependency> subpassDependency = {{
        VK_SUBPASS_EXTERNAL,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlags(),
        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
    }};

    vk::RenderPassCreateInfo renderPassCreateInfo = {
        vk::RenderPassCreateFlags(),
        boost::numeric_cast<uint32_t>(attachmentDescriptions.size()), attachmentDescriptions.data(),
        boost::numeric_cast<uint32_t>(subpassDescriptions.size()), subpassDescriptions.data(),
        boost::numeric_cast<uint32_t>(subpassDependency.size()), subpassDependency.data()
    };

    firstRenderPass = device->intrinsic.createRenderPass(renderPassCreateInfo);
    attachmentDescriptions[0].loadOp = vk::AttachmentLoadOp::eClear;
    followUpRenderPass = device->intrinsic.createRenderPass(renderPassCreateInfo);
}

void Window::teardownRenderPasses(void)
{
    device->intrinsic.destroy(firstRenderPass);
    device->intrinsic.destroy(followUpRenderPass);
}

void Window::buildPipelines(void)
{
    backingPipeline->buildPipeline(firstRenderPass, swapchainCreateInfo.imageExtent);
}

void Window::teardownPipelines(void)
{
    backingPipeline->teardownPipeline();
}

void Window::buildSemaphores(void)
{
    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
    imageAvailableSemaphore = device->intrinsic.createSemaphore(semaphoreCreateInfo, nullptr);

    // This fence is used to wait for the Window and its Pipelines to be idle.
    // It should therefor be signed at the start so that when no rendering has been
    // done it is still idle.
    auto fenceCreateInfo = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
    renderFinishedFence = device->intrinsic.createFence(fenceCreateInfo, nullptr);
}

void Window::teardownSemaphores(void)
{
    device->intrinsic.destroy(imageAvailableSemaphore);
    device->intrinsic.destroy(renderFinishedFence);
}


bool Window::render(bool blockOnVSync)
{
    uint32_t imageIndex;
    uint64_t timeout = blockOnVSync ? std::numeric_limits<uint64_t>::max() : 0;

    LOG_INFO("Render.");

    auto result = device->intrinsic.acquireNextImageKHR(swapchain, timeout, imageAvailableSemaphore, vk::Fence(), &imageIndex);
    switch (result) {
    case vk::Result::eSuccess:
        break;
    case vk::Result::eSuboptimalKHR: /* FALLTHROUGH */
    case vk::Result::eErrorOutOfDateKHR:
        // SwapChain needs to be rebuild.
        return false;
    case vk::Result::eTimeout:
        // Don't render, we didn't receive an image.
        return true;
    default:
        BOOST_THROW_EXCEPTION(WindowSwapChainError());
    }

    vk::Semaphore renderFinishedSemaphores[] = {
        backingPipeline->render(imageIndex, imageAvailableSemaphore)
    };

    // Make a fence that should be signaled when all drawing is finished.
    device->intrinsic.waitIdle();
    //device->intrinsic.waitForFences(1, &renderFinishedFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    device->intrinsic.resetFences(1, &renderFinishedFence);
    device->graphicQueue->intrinsic.submit(0, nullptr, renderFinishedFence);

    auto presentInfo = vk::PresentInfoKHR(
        1, renderFinishedSemaphores,
        1, &swapchain,
        &imageIndex
    );

    // Pass present info as a pointer to get the non-throw version.
    result = device->presentQueue->intrinsic.presentKHR(&presentInfo);
    switch (result) {
    case vk::Result::eSuccess:
        break;
    case vk::Result::eSuboptimalKHR: /* FALLTHROUGH */
    case vk::Result::eErrorOutOfDateKHR:
        // SwapChain needs to be rebuild.
        return false;
    default:
        BOOST_THROW_EXCEPTION(WindowSwapChainError());
    }
    return true;
}


}}}
