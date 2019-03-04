//
//  Window.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Window.hpp"
#include "Device.hpp"
#include "TTauri/Toolkit/Logging.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

using namespace std;

void Window::buildSwapchain(void)
{
    surfaceCapabilities = device->physicalIntrinsic.getSurfaceCapabilitiesKHR(intrinsic);

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

    auto imageCount = std::clamp(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

    uint32_t imageArrayLayers = 1;
    vk::Bool32 clipped = VK_TRUE;

    vk::Extent2D imageExtent = surfaceCapabilities.currentExtent;
    if (imageExtent.width == std::numeric_limits<uint32_t>::max() or imageExtent.height == std::numeric_limits<uint32_t>::max()) {
        imageExtent.width = std::clamp(displayRectangle.extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        imageExtent.height = std::clamp(displayRectangle.extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
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

    LOG_INFO("Building swap chain");
    LOG_INFO(" - extent=%i x %i") % swapchainCreateInfo.imageExtent.width % swapchainCreateInfo.imageExtent.height;
    LOG_INFO(" - colorSpace=%s, format=%s") % vk::to_string(swapchainCreateInfo.imageColorSpace) % vk::to_string(swapchainCreateInfo.imageFormat);
    LOG_INFO(" - presentMode=%s, imageCount=%i") % vk::to_string(swapchainCreateInfo.presentMode) % swapchainCreateInfo.minImageCount;

    swapchain = device->intrinsic.createSwapchainKHR(swapchainCreateInfo);
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

        auto framebuffer = device->intrinsic.createFramebuffer(framebufferCreateInfo);
        swapchainFramebuffers.push_back(framebuffer);
    }
}

void Window::teardownFramebuffers(void)
{
    for (auto frameBuffer: swapchainFramebuffers) {
        device->intrinsic.destroy(frameBuffer);
    }
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
    backingPipeline = std::make_shared<BackingPipeline>(this, firstRenderPass);
    backingPipeline->initialize();
}

void Window::teardownPipelines(void)
{
    backingPipeline = nullptr;
}

void Window::buildSemaphores(void)
{
    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();

    imageAvailableSemaphore = device->intrinsic.createSemaphore(semaphoreCreateInfo, nullptr);
}

void Window::teardownSemaphores(void)
{
    device->intrinsic.destroy(imageAvailableSemaphore);
}

void Window::buildSwapchainAndPipeline(void)
{
    boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

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
    boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

    if (state == WindowState::READY_TO_DRAW) {
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
            boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
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
            boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
            if (state == WindowState::LINKED_TO_DEVICE) {
                this->device = nullptr;
                state = WindowState::NO_DEVICE;
                
            } else {
                BOOST_THROW_EXCEPTION(WindowStateError());
            }
        }
    }
}

void Window::render(void)
{
    auto imageIndexValue = device->intrinsic.acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, vk::Fence());
    uint32_t imageIndex = imageIndexValue.value;

    vk::Semaphore renderFinishedSemaphores[] = {
        backingPipeline->render(imageIndex, imageAvailableSemaphore)
    };

    auto presentInfo = vk::PresentInfoKHR(
        1, renderFinishedSemaphores,
        1, &swapchain,
        &imageIndex
    );

    device->presentQueue->intrinsic.presentKHR(presentInfo);
}

void Window::frameUpdate(uint64_t nowTimestamp, uint64_t outputTimestamp)
{
    if (stateMutex.try_lock_shared()) {
        if (state == WindowState::READY_TO_DRAW) {
            render();
        }
        stateMutex.unlock_shared();
    }
}

}}}
