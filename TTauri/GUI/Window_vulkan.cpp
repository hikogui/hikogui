// Copyright 2019 Pokitec
// All rights reserved.

#include "Window_vulkan.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "WindowWidget.hpp"
#include "TTauri/all.hpp"
#include <boost/numeric/conversion/cast.hpp>
#include <vector>

namespace TTauri::GUI {

using namespace std;

Window_vulkan::Window_vulkan(const std::shared_ptr<WindowDelegate> delegate, const std::string title) :
    Window_base(move(delegate), move(title))
{
}

Window_vulkan::~Window_vulkan()
{
}

void Window_vulkan::initialize()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    Window_base::initialize();
    auto window = std::dynamic_pointer_cast<Window>(shared_from_this());
    assert(window);
    imagePipeline = TTauri::make_shared<PipelineImage::PipelineImage>(window);
}

void Window_vulkan::waitIdle()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto vulkanDevice = device.lock();
    vulkanDevice->waitForFences({ renderFinishedFence }, VK_TRUE, std::numeric_limits<uint64_t>::max());
    vulkanDevice->waitIdle();
    LOG_INFO("/waitIdle");
}

std::optional<uint32_t> Window_vulkan::acquireNextImageFromSwapchain()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    // swapchain, fence & imageAvailableSemaphore must be externally synchronized.
    uint32_t imageIndex = 0;
    //LOG_DEBUG("acquireNextImage '%s'") % title;
    let result = vulkanDevice->acquireNextImageKHR(swapchain, 0, imageAvailableSemaphore, vk::Fence(), &imageIndex);
    //LOG_DEBUG("acquireNextImage %i") % imageIndex;

    switch (result) {
    case vk::Result::eSuccess:
        return {imageIndex};

    case vk::Result::eSuboptimalKHR:
        LOG_INFO("acquireNextImageKHR() eSuboptimalKHR");
        state = State::SWAPCHAIN_LOST;
        return {};

    case vk::Result::eErrorOutOfDateKHR:
        LOG_INFO("acquireNextImageKHR() eErrorOutOfDateKHR");
        state = State::SWAPCHAIN_LOST;
        return {};

    case vk::Result::eErrorSurfaceLostKHR:
        LOG_INFO("acquireNextImageKHR() eErrorSurfaceLostKHR");
        state = State::SURFACE_LOST;
        return {};

    case vk::Result::eTimeout:
        // Don't render, we didn't receive an image.
        LOG_INFO("acquireNextImageKHR() eTimeout");
        return {};

    default:
        BOOST_THROW_EXCEPTION(Window_base::SwapChainError());
    }
}

void Window_vulkan::presentImageToQueue(uint32_t imageIndex, vk::Semaphore renderFinishedSemaphore)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    vector<vk::Semaphore> const renderFinishedSemaphores = { renderFinishedSemaphore };
    vector<vk::SwapchainKHR> const presentSwapchains = { swapchain };
    vector<uint32_t> const presentImageIndices = { imageIndex };
    BOOST_ASSERT(presentSwapchains.size() == presentImageIndices.size());

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    try {
        //LOG_DEBUG("presentQueue %i") % presentImageIndices.at(0);

        let result = vulkanDevice->presentQueue.presentKHR({
            boost::numeric_cast<uint32_t>(renderFinishedSemaphores.size()), renderFinishedSemaphores.data(),
            boost::numeric_cast<uint32_t>(presentSwapchains.size()), presentSwapchains.data(), presentImageIndices.data()
            });

        switch (result) {
        case vk::Result::eSuccess:
            return;

        case vk::Result::eSuboptimalKHR:
            LOG_INFO("presentKHR() eSuboptimalKHR");
            state = State::SWAPCHAIN_LOST;
            return;

        default:
            BOOST_THROW_EXCEPTION(Window_base::SwapChainError());
            return;
        }

    }
    catch (const vk::OutOfDateKHRError&) {
        LOG_INFO("presentKHR() eErrorOutOfDateKHR");
        state = State::SWAPCHAIN_LOST;
        return;
    }
    catch (const vk::SurfaceLostKHRError&) {
        LOG_INFO("presentKHR() eErrorSurfaceLostKHR");
        state = State::SURFACE_LOST;
        return;
    }

    // Make sure that resources are released by Vulkan by calling
    vulkanDevice->waitIdle();
}

void Window_vulkan::build()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    if (state == State::NO_WINDOW) {
    }

    if (state == State::NO_DEVICE) {
        if (!device.expired()) {
            imagePipeline->buildForNewDevice();
            state = State::NO_SURFACE;
        }
    }

    if (state == State::NO_SURFACE) {
        if (!buildSurface()) {
            state = State::DEVICE_LOST;
            return;
        }
        imagePipeline->buildForNewSurface();
        state = State::NO_SWAPCHAIN;
    }

    if (state == State::NO_SWAPCHAIN) {
        if (!readSurfaceExtent()) {
            // Minimized window, can not build a new swapchain.
            return;
        }

        let s = buildSwapchain();
        if (s != State::READY_TO_RENDER) {
            state = s;
            return;
        }

        if (!checkSurfaceExtent()) {
            // Window has changed during swapchain creation, it is in a inconsistant bad state.
            // This is a bug in the Vulkan specification.
            teardownSwapchain();
            return;
        }
        buildRenderPasses();
        buildFramebuffers();
        buildSemaphores();
        imagePipeline->buildForNewSwapchain(firstRenderPass, swapchainImageExtent, nrSwapchainImages);

        windowChangedSize({ swapchainImageExtent.width, swapchainImageExtent.height });
        state = State::READY_TO_RENDER;
    }
}

void Window_vulkan::teardown()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    if (state >= State::SWAPCHAIN_LOST) {
        waitIdle();
        imagePipeline->teardownForSwapchainLost();
        teardownSemaphores();
        teardownFramebuffers();
        teardownRenderPasses();
        teardownSwapchain();
    }
    if (state >= State::SURFACE_LOST) {
        imagePipeline->teardownForSurfaceLost();
        teardownSurface();
    }
    if (state >= State::DEVICE_LOST) {
        imagePipeline->teardownForDeviceLost();
        teardownDevice();
    }
    if (state >= State::WINDOW_LOST) {
        imagePipeline->teardownForWindowLost();
        teardownWindow();
    }

    switch (state) {
    case State::WINDOW_LOST: state = State::NO_WINDOW; break;
    case State::DEVICE_LOST: state = State::NO_DEVICE; break;
    case State::SURFACE_LOST: state = State::NO_SURFACE; break;
    case State::SWAPCHAIN_LOST: state = State::NO_SWAPCHAIN; break;
    }
}

void Window_vulkan::render()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    rebuild();

    if (state != State::READY_TO_RENDER) {
        return;
    }

    let imageIndex = acquireNextImageFromSwapchain();
    if (!imageIndex) {
        // No image is ready to be rendered, yet, possibly because our vertical sync function
        // is not working correctly.
        return;
    }

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    // Wait until previous rendering has finished, before the next rendering.
    // XXX maybe use one for each swapchain image or go to single command buffer.
    vulkanDevice->waitForFences({ renderFinishedFence }, VK_TRUE, std::numeric_limits<uint64_t>::max());

    // Unsignal the fence so we will not modify/destroy the command buffers during rendering.
    vulkanDevice->resetFences({ renderFinishedFence });

    let renderFinishedSemaphore = imagePipeline->render(imageIndex.value(), imageAvailableSemaphore);

    // Signal the fence when all rendering has finished on the graphics queue.
    // When the fence is signaled we can modify/destroy the command buffers.
    vulkanDevice->graphicsQueue.submit(0, nullptr, renderFinishedFence);

    presentImageToQueue(imageIndex.value(), renderFinishedSemaphore);
}

std::tuple<uint32_t, vk::Extent2D> Window_vulkan::getImageCountAndExtent()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    surfaceCapabilities = lock_dynamic_cast<Device_vulkan>(device)->getSurfaceCapabilitiesKHR(intrinsic);

    let currentExtentSet =
        (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) &&
        (surfaceCapabilities.currentExtent.height != std::numeric_limits<uint32_t>::max());

    if (!currentExtentSet) {
        LOG_WARNING("getSurfaceCapabilitiesKHR() does not supply currentExtent");
    }

    uint32_t const imageCount = surfaceCapabilities.maxImageCount ?
        std::clamp(defaultNumberOfSwapchainImages, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount) :
        std::max(defaultNumberOfSwapchainImages, surfaceCapabilities.minImageCount);

    vk::Extent2D const imageExtent = currentExtentSet ?
        surfaceCapabilities.currentExtent :
        (vk::Extent2D{
            std::clamp(
                static_cast<uint32_t>(windowRectangle.extent.width()),
                surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width
            ),
            std::clamp(
                static_cast<uint32_t>(windowRectangle.extent.height()),
                surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height
            )
        });

    return { imageCount, imageExtent };
}

bool Window_vulkan::readSurfaceExtent()
{
    try {
        std::tie(nrSwapchainImages, swapchainImageExtent) = getImageCountAndExtent();

    } catch (const vk::SurfaceLostKHRError&) {
        state = State::SURFACE_LOST;
        return false;
    }

    return swapchainImageExtent.width > 0 && swapchainImageExtent.height > 0;
}

bool Window_vulkan::checkSurfaceExtent()
{
    try {
        let [nrImages, extent] = getImageCountAndExtent();
        return (nrImages == nrSwapchainImages) && (extent == swapchainImageExtent);

    } catch (const vk::SurfaceLostKHRError&) {
        state = State::SURFACE_LOST;
        return false;
    }
}

void Window_vulkan::buildDevice()
{
}

bool Window_vulkan::buildSurface()
{
    intrinsic = getSurface();

    return device.lock()->score(intrinsic) > 0;
}

Window_base::State Window_vulkan::buildSwapchain()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    LOG_INFO("Building swap chain");

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    let sharingMode = vulkanDevice->graphicsQueueFamilyIndex == vulkanDevice->presentQueueFamilyIndex ?
        vk::SharingMode::eExclusive :
        vk::SharingMode::eConcurrent;

    vector<uint32_t> const sharingQueueFamilyAllIndices = { vulkanDevice->graphicsQueueFamilyIndex, vulkanDevice->presentQueueFamilyIndex };
    vector<uint32_t> const sharingQueueFamilyNoneIndices = {};

    let sharingQueueFamilyIndices = sharingMode == vk::SharingMode::eConcurrent ? sharingQueueFamilyAllIndices : sharingQueueFamilyNoneIndices;

    swapchainImageFormat = vulkanDevice->bestSurfaceFormat;
    vk::SwapchainCreateInfoKHR swapchainCreateInfo{
        vk::SwapchainCreateFlagsKHR(),
        intrinsic,
        nrSwapchainImages,
        swapchainImageFormat.format,
        swapchainImageFormat.colorSpace,
        swapchainImageExtent,
        1, // imageArrayLayers
        vk::ImageUsageFlagBits::eColorAttachment,
        sharingMode,
        boost::numeric_cast<uint32_t>(sharingQueueFamilyIndices.size()),
        sharingQueueFamilyIndices.data(),
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vulkanDevice->bestSurfacePresentMode,
        VK_TRUE, // clipped
        nullptr
    };
        
    vk::Result const result = vulkanDevice->createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapchain);
    switch (result) {
    case vk::Result::eSuccess:
        break;

    case vk::Result::eErrorSurfaceLostKHR:
        return State::SURFACE_LOST;

    default:
        BOOST_THROW_EXCEPTION(Window_base::SwapChainError());
    }

    LOG_INFO("Finished building swap chain");
    LOG_INFO(" - extent=%i x %i") % swapchainCreateInfo.imageExtent.width % swapchainCreateInfo.imageExtent.height;
    LOG_INFO(" - colorSpace=%s, format=%s") % vk::to_string(swapchainCreateInfo.imageColorSpace) % vk::to_string(swapchainCreateInfo.imageFormat);
    LOG_INFO(" - presentMode=%s, imageCount=%i") % vk::to_string(swapchainCreateInfo.presentMode) % swapchainCreateInfo.minImageCount;
    return State::READY_TO_RENDER;
}

void Window_vulkan::teardownSwapchain()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    lock_dynamic_cast<Device_vulkan>(device)->destroy(swapchain);
}

void Window_vulkan::buildFramebuffers()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    swapchainImages = vulkanDevice->getSwapchainImagesKHR(swapchain);
    for (auto image : swapchainImages) {
        auto imageView = vulkanDevice->createImageView({
            vk::ImageViewCreateFlags(),
            image,
            vk::ImageViewType::e2D,
            swapchainImageFormat.format,
            vk::ComponentMapping(),
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        });

        swapchainImageViews.push_back(imageView);

        std::vector<vk::ImageView> attachments = { imageView };

        auto framebuffer = vulkanDevice->createFramebuffer({
            vk::FramebufferCreateFlags(),
            firstRenderPass,
            boost::numeric_cast<uint32_t>(attachments.size()),
            attachments.data(),
            swapchainImageExtent.width,
            swapchainImageExtent.height,
            1 // layers
        });
        swapchainFramebuffers.push_back(framebuffer);
    }

    BOOST_ASSERT(swapchainImageViews.size() == swapchainImages.size());
    BOOST_ASSERT(swapchainFramebuffers.size() == swapchainImages.size());
}

void Window_vulkan::teardownFramebuffers()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    for (auto frameBuffer : swapchainFramebuffers) {
        vulkanDevice->destroy(frameBuffer);
    }
    swapchainFramebuffers.clear();

    for (auto imageView : swapchainImageViews) {
        vulkanDevice->destroy(imageView);
    }
    swapchainImageViews.clear();
}

void Window_vulkan::buildRenderPasses()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    std::vector<vk::AttachmentDescription> attachmentDescriptions = { {
        vk::AttachmentDescriptionFlags(),
        swapchainImageFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
        vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
        vk::ImageLayout::eUndefined, // initialLayout
        vk::ImageLayout::ePresentSrcKHR // finalLayout
    } };

    std::vector<vk::AttachmentReference> const inputAttachmentReferences = {};

    std::vector<vk::AttachmentReference> const colorAttachmentReferences = { { 0, vk::ImageLayout::eColorAttachmentOptimal } };

    std::vector<vk::SubpassDescription> const subpassDescriptions = { {
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        boost::numeric_cast<uint32_t>(inputAttachmentReferences.size()),
        inputAttachmentReferences.data(),
        boost::numeric_cast<uint32_t>(colorAttachmentReferences.size()),
        colorAttachmentReferences.data()
    } };

    std::vector<vk::SubpassDependency> const subpassDependency = { {
        VK_SUBPASS_EXTERNAL,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlags(),
        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
    } };

    vk::RenderPassCreateInfo const renderPassCreateInfo = {
        vk::RenderPassCreateFlags(),
        boost::numeric_cast<uint32_t>(attachmentDescriptions.size()),
        attachmentDescriptions.data(),
        boost::numeric_cast<uint32_t>(subpassDescriptions.size()),
        subpassDescriptions.data(),
        boost::numeric_cast<uint32_t>(subpassDependency.size()),
        subpassDependency.data()
    };

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    firstRenderPass = vulkanDevice->createRenderPass(renderPassCreateInfo);
    attachmentDescriptions.at(0).loadOp = vk::AttachmentLoadOp::eClear;
    followUpRenderPass = vulkanDevice->createRenderPass(renderPassCreateInfo);
}

void Window_vulkan::teardownRenderPasses()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);
    vulkanDevice->destroy(firstRenderPass);
    vulkanDevice->destroy(followUpRenderPass);
}

void Window_vulkan::buildSemaphores()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    imageAvailableSemaphore = vulkanDevice->createSemaphore({});

    // This fence is used to wait for the Window and its Pipelines to be idle.
    // It should therefor be signed at the start so that when no rendering has been
    // done it is still idle.
    renderFinishedFence = vulkanDevice->createFence({ vk::FenceCreateFlagBits::eSignaled });
}

void Window_vulkan::teardownSemaphores()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);
    vulkanDevice->destroy(imageAvailableSemaphore);
    vulkanDevice->destroy(renderFinishedFence);
}


void Window_vulkan::teardownSurface()
{
    instance->destroySurfaceKHR(intrinsic);
}

void Window_vulkan::teardownDevice()
{
        device.reset();
}

void Window_vulkan::teardownWindow()
{

}

}
