// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Window_vulkan.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/Instance.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/GUI/PipelineImage.hpp"
#include "TTauri/GUI/PipelineFlat.hpp"
#include "TTauri/GUI/PipelineBox.hpp"
#include "TTauri/GUI/PipelineSDF.hpp"
#include "TTauri/Foundation/trace.hpp"
#include <vector>

namespace TTauri::GUI {

using namespace std;

Window_vulkan::Window_vulkan(const std::shared_ptr<WindowDelegate> delegate, const std::string title) :
    Window_base(move(delegate), move(title)), nrSwapchainImages(0), swapchainImageFormat()
{
}

Window_vulkan::~Window_vulkan()
{
}

void Window_vulkan::initialize()
{
    std::scoped_lock lock(GUI_globals->mutex);

    Window_base::initialize();
    flatPipeline = std::make_unique<PipelineFlat::PipelineFlat>(dynamic_cast<Window &>(*this));
    boxPipeline = std::make_unique<PipelineBox::PipelineBox>(dynamic_cast<Window &>(*this));
    SDFPipeline = std::make_unique<PipelineSDF::PipelineSDF>(dynamic_cast<Window &>(*this));
    imagePipeline = std::make_unique<PipelineImage::PipelineImage>(dynamic_cast<Window &>(*this));
}

void Window_vulkan::waitIdle()
{
    std::scoped_lock lock(GUI_globals->mutex);

    ttauri_assert(device);
    device->waitForFences({ renderFinishedFence }, VK_TRUE, std::numeric_limits<uint64_t>::max());
    device->waitIdle();
    LOG_INFO("/waitIdle");
}

std::optional<uint32_t> Window_vulkan::acquireNextImageFromSwapchain()
{
    std::scoped_lock lock(GUI_globals->mutex);

    // swap chain, fence & imageAvailableSemaphore must be externally synchronized.
    uint32_t frameBufferIndex = 0;
    //LOG_DEBUG("acquireNextImage '{}'", title);
    ttauri_assert(device);
    let result = device->acquireNextImageKHR(swapchain, 0, imageAvailableSemaphore, vk::Fence(), &frameBufferIndex);
    //LOG_DEBUG("acquireNextImage {}", frameBufferIndex);

    switch (result) {
    case vk::Result::eSuccess:
        return {frameBufferIndex};

    case vk::Result::eSuboptimalKHR:
        LOG_INFO("acquireNextImageKHR() eSuboptimalKHR");
        state = State::SwapchainLost;
        return {};

    case vk::Result::eErrorOutOfDateKHR:
        LOG_INFO("acquireNextImageKHR() eErrorOutOfDateKHR");
        state = State::SwapchainLost;
        return {};

    case vk::Result::eErrorSurfaceLostKHR:
        LOG_INFO("acquireNextImageKHR() eErrorSurfaceLostKHR");
        state = State::SurfaceLost;
        return {};

    case vk::Result::eTimeout:
        // Don't render, we didn't receive an image.
        LOG_INFO("acquireNextImageKHR() eTimeout");
        return {};

    default:
        TTAURI_THROW(gui_error("Unknown result from acquireNextImageKHR()")
            .set<"vk_result"_tag>(to_string(result))
        );
    }
}

void Window_vulkan::presentImageToQueue(uint32_t frameBufferIndex, vk::Semaphore renderFinishedSemaphore)
{
    ttauri_assert(device);

    std::scoped_lock lock(GUI_globals->mutex);

    std::array<vk::Semaphore, 1> const renderFinishedSemaphores = { renderFinishedSemaphore };
    std::array<vk::SwapchainKHR, 1> const presentSwapchains = { swapchain };
    std::array<uint32_t, 1> const presentImageIndices = { frameBufferIndex };
    BOOST_ASSERT(presentSwapchains.size() == presentImageIndices.size());

    try {
        //LOG_DEBUG("presentQueue {}", presentImageIndices.at(0));
        let result = device->presentQueue.presentKHR({
            numeric_cast<uint32_t>(renderFinishedSemaphores.size()), renderFinishedSemaphores.data(),
            numeric_cast<uint32_t>(presentSwapchains.size()), presentSwapchains.data(), presentImageIndices.data()
        });

        switch (result) {
        case vk::Result::eSuccess:
            return;

        case vk::Result::eSuboptimalKHR:
            LOG_INFO("presentKHR() eSuboptimalKHR");
            state = State::SwapchainLost;
            return;

        default:
            TTAURI_THROW(gui_error("Unknown result from presentKHR()")
                .set<"vk_result"_tag>(to_string(result))
            );
        }

    }
    catch (const vk::OutOfDateKHRError&) {
        LOG_INFO("presentKHR() eErrorOutOfDateKHR");
        state = State::SwapchainLost;
        return;
    }
    catch (const vk::SurfaceLostKHRError&) {
        LOG_INFO("presentKHR() eErrorSurfaceLostKHR");
        state = State::SurfaceLost;
        return;
    }

    // Make sure that resources are released by Vulkan by calling
    device->waitIdle();
}

void Window_vulkan::build()
{
    std::scoped_lock lock(GUI_globals->mutex);

    if (state == State::NoDevice) {
        if (device) {
            flatPipeline->buildForNewDevice(device);
            boxPipeline->buildForNewDevice(device);
            imagePipeline->buildForNewDevice(device);
            SDFPipeline->buildForNewDevice(device);
            state = State::NoSurface;
        }
    }

    if (state == State::NoSurface) {
        if (!buildSurface()) {
            state = State::DeviceLost;
            return;
        }
        flatPipeline->buildForNewSurface();
        boxPipeline->buildForNewSurface();
        imagePipeline->buildForNewSurface();
        SDFPipeline->buildForNewSurface();
        state = State::NoSwapchain;
    }

    if (state == State::NoSwapchain) {
        if (!readSurfaceExtent()) {
            // Minimized window, can not build a new swap chain.
            return;
        }

        let s = buildSwapchain();
        if (s != State::ReadyToRender) {
            state = s;
            return;
        }

        if (!checkSurfaceExtent()) {
            // Window has changed during swap chain creation, it is in a inconsistent bad state.
            // This is a bug in the Vulkan specification.
            teardownSwapchain();
            return;
        }
        buildRenderPasses();
        buildFramebuffers();
        buildSemaphores();
        flatPipeline->buildForNewSwapchain(firstRenderPass, swapchainImageExtent);
        boxPipeline->buildForNewSwapchain(followUpRenderPass, swapchainImageExtent);
        imagePipeline->buildForNewSwapchain(followUpRenderPass, swapchainImageExtent);
        SDFPipeline->buildForNewSwapchain(lastRenderPass, swapchainImageExtent);

        windowChangedSize({
            numeric_cast<float>(swapchainImageExtent.width),
            numeric_cast<float>(swapchainImageExtent.height)
        });
        state = State::ReadyToRender;
    }
}

void Window_vulkan::teardown()
{
    std::scoped_lock lock(GUI_globals->mutex);
    auto nextState = state;

    if (state >= State::SwapchainLost) {
        LOG_INFO("Tearing down because the window lost the swapchain.");
        waitIdle();
        imagePipeline->teardownForSwapchainLost();
        flatPipeline->teardownForSwapchainLost();
        boxPipeline->teardownForSwapchainLost();
        SDFPipeline->teardownForSwapchainLost();
        teardownSemaphores();
        teardownFramebuffers();
        teardownRenderPasses();
        teardownSwapchain();
        nextState = State::NoSwapchain;

        if (state >= State::SurfaceLost) {
            LOG_INFO("Tearing down because the window lost the drawable surface.");
            imagePipeline->teardownForSurfaceLost();
            flatPipeline->teardownForSurfaceLost();
            boxPipeline->teardownForSurfaceLost();
            SDFPipeline->teardownForSurfaceLost();
            teardownSurface();
            nextState = State::NoSurface;

            if (state >= State::DeviceLost) {
                LOG_INFO("Tearing down because the window lost the vulkan device.");

                imagePipeline->teardownForDeviceLost();
                flatPipeline->teardownForDeviceLost();
                boxPipeline->teardownForDeviceLost();
                SDFPipeline->teardownForDeviceLost();
                teardownDevice();
                nextState = State::NoDevice;

                if (state >= State::WindowLost) {
                    LOG_INFO("Tearing down because the window doesn't exist anymore.");

                    imagePipeline->teardownForWindowLost();
                    flatPipeline->teardownForWindowLost();
                    boxPipeline->teardownForWindowLost();
                    SDFPipeline->teardownForWindowLost();
                    // State::NO_WINDOW will be set after finishing delegate.closingWindow() on the mainThread
                    closingWindow();
                }
            }
        }
    }
    state = nextState;
}

void Window_vulkan::render()
{
    auto tr = trace<"window_render"_tag, "resizing"_tag, "state"_tag, "frame_buffer"_tag>();
    auto lock = std::scoped_lock{GUI_globals->mutex};

    // While resizing lower the frame rate to reduce CPU usage.
    tr.set<"resizing"_tag>(resizing);
    if (resizing && (frameCount++ % resizeFrameRateDivider) != 0) {
        return;
    }

    // Tear down then buildup from the vulkan objects that where invalid.
    teardown();
    build();

    tr.set<"state"_tag>(static_cast<int>(state));
    if (state != State::ReadyToRender) {
        return;
    }

    let optionalFrameBufferIndex = acquireNextImageFromSwapchain();
    if (!optionalFrameBufferIndex) {
        // No image is ready to be rendered, yet, possibly because our vertical sync function
        // is not working correctly.
        return;
    }
    let frameBufferIndex = *optionalFrameBufferIndex;
    let frameBuffer = swapchainFramebuffers.at(frameBufferIndex);

    tr.set<"frame_buffer"_tag>(frameBufferIndex);

    // Wait until previous rendering has finished, before the next rendering.
    // XXX maybe use one for each swap chain image or go to single command buffer.
    ttauri_assert(device);
    device->waitForFences({ renderFinishedFence }, VK_TRUE, std::numeric_limits<uint64_t>::max());

    // Unsignal the fence so we will not modify/destroy the command buffers during rendering.
    device->resetFences({ renderFinishedFence });

    // Update the widgets before the pipelines need their vertices.
    widget->update(
        widget->modified(),
        flatPipeline->vertexBufferData.clear(),
        boxPipeline->vertexBufferData.clear(),
        imagePipeline->vertexBufferData.clear(),
        SDFPipeline->vertexBufferData.clear()
    );

    // The flat pipeline goes first, because it will not have anti-aliasing, and often it needs to be drawn below
    // images with alpha-channel.
    let flatPipelineFinishedSemaphore = flatPipeline->render(frameBuffer, imageAvailableSemaphore);
    let boxPipelineFinishedSemaphore = boxPipeline->render(frameBuffer, flatPipelineFinishedSemaphore);
    let imagePipelineFinishedSemaphore = imagePipeline->render(frameBuffer, boxPipelineFinishedSemaphore);
    let renderFinishedSemaphore = SDFPipeline->render(frameBuffer, imagePipelineFinishedSemaphore);

    // Signal the fence when all rendering has finished on the graphics queue.
    // When the fence is signaled we can modify/destroy the command buffers.
    device->graphicsQueue.submit(0, nullptr, renderFinishedFence);

    presentImageToQueue(frameBufferIndex, renderFinishedSemaphore);

    // Do an early tear down of invalid vulkan objects.
    teardown();
}

std::tuple<uint32_t, vk::Extent2D> Window_vulkan::getImageCountAndExtent()
{
    std::scoped_lock lock(GUI_globals->mutex);

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    ttauri_assert(device);
    surfaceCapabilities = device->getSurfaceCapabilitiesKHR(intrinsic);

    LOG_INFO("minimumExtent=({}, {}), maximumExtent=({}, {}), currentExtent=({}, {}), osExtent=({}, {})",
        surfaceCapabilities.minImageExtent.width, surfaceCapabilities.minImageExtent.height,
        surfaceCapabilities.maxImageExtent.width, surfaceCapabilities.maxImageExtent.height,
        surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height,
        OSWindowRectangle.extent.width(), OSWindowRectangle.extent.height()
    );

    let currentExtentSet =
        (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) &&
        (surfaceCapabilities.currentExtent.height != std::numeric_limits<uint32_t>::max());

    if (!currentExtentSet) {
        LOG_WARNING("getSurfaceCapabilitiesKHR() does not supply currentExtent");
    }

    uint32_t const imageCount = surfaceCapabilities.maxImageCount ?
        std::clamp(GUI_globals->defaultNumberOfSwapchainImages, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount) :
        std::max(GUI_globals->defaultNumberOfSwapchainImages, surfaceCapabilities.minImageCount);

    vk::Extent2D const imageExtent = currentExtentSet ?
        surfaceCapabilities.currentExtent :
        (vk::Extent2D{
            std::clamp(
                static_cast<uint32_t>(OSWindowRectangle.extent.width()),
                surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width
            ),
            std::clamp(
                static_cast<uint32_t>(OSWindowRectangle.extent.height()),
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
        state = State::SurfaceLost;
        return false;
    }

    if (
        swapchainImageExtent.width < minimumWindowExtent.width() ||
        swapchainImageExtent.height < minimumWindowExtent.height()
    ) {
        // Due to vulkan surface being extended across the window decoration;
        // On Windows 10 the swapchain-extent on a minimized window is no longer 0x0 instead
        // it is 160x28 pixels.

        //LOG_INFO("Window too small to draw current=({}, {}), minimum=({}, {})",
        //    swapchainImageExtent.width, swapchainImageExtent.height,
        //    minimumWindowExtent.width(), minimumWindowExtent.height()
        //);
        return false;
    }

    if (
        swapchainImageExtent.width > maximumWindowExtent.width() ||
        swapchainImageExtent.height < maximumWindowExtent.height()
        ) {
        LOG_ERROR("Window too large to draw current=({}, {}), maximum=({}, {})",
            swapchainImageExtent.width, swapchainImageExtent.height,
            maximumWindowExtent.width(), maximumWindowExtent.height()
        );
        return false;
    }

    return true;
}

bool Window_vulkan::checkSurfaceExtent()
{
    try {
        let [nrImages, extent] = getImageCountAndExtent();
        return (nrImages == nrSwapchainImages) && (extent == swapchainImageExtent);

    } catch (const vk::SurfaceLostKHRError&) {
        state = State::SurfaceLost;
        return false;
    }
}

void Window_vulkan::buildDevice()
{
}

bool Window_vulkan::buildSurface()
{
    intrinsic = getSurface();

    ttauri_assert(device);
    return device->score(intrinsic) > 0;
}

Window_base::State Window_vulkan::buildSwapchain()
{
    ttauri_assert(device);

    std::scoped_lock lock(GUI_globals->mutex);

    LOG_INFO("Building swap chain");

    let sharingMode = device->graphicsQueueFamilyIndex == device->presentQueueFamilyIndex ?
        vk::SharingMode::eExclusive :
        vk::SharingMode::eConcurrent;

    std::array<uint32_t, 2> const sharingQueueFamilyAllIndices = { device->graphicsQueueFamilyIndex, device->presentQueueFamilyIndex };

    swapchainImageFormat = device->bestSurfaceFormat;
    vk::SwapchainCreateInfoKHR swapchainCreateInfo{
        vk::SwapchainCreateFlagsKHR(),
        intrinsic,
        static_cast<uint32_t>(nrSwapchainImages),
        swapchainImageFormat.format,
        swapchainImageFormat.colorSpace,
        swapchainImageExtent,
        1, // imageArrayLayers
        vk::ImageUsageFlagBits::eColorAttachment,
        sharingMode,
        sharingMode == vk::SharingMode::eConcurrent ? numeric_cast<uint32_t>(sharingQueueFamilyAllIndices.size()) : 0,
        sharingMode == vk::SharingMode::eConcurrent ? sharingQueueFamilyAllIndices.data() : nullptr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        device->bestSurfacePresentMode,
        VK_TRUE, // clipped
        nullptr
    };
        
    vk::Result const result = device->createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapchain);
    switch (result) {
    case vk::Result::eSuccess:
        break;

    case vk::Result::eErrorSurfaceLostKHR:
        return State::SurfaceLost;

    default:
        TTAURI_THROW(gui_error("Unknown result from createSwapchainKHR()")
            .set<"vk_result"_tag>(to_string(result))
        );
    }

    LOG_INFO("Finished building swap chain");
    LOG_INFO(" - extent=({}, {})", swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height);
    LOG_INFO(" - colorSpace={}, format={}", vk::to_string(swapchainCreateInfo.imageColorSpace), vk::to_string(swapchainCreateInfo.imageFormat));
    LOG_INFO(" - presentMode={}, imageCount={}", vk::to_string(swapchainCreateInfo.presentMode), swapchainCreateInfo.minImageCount);
    return State::ReadyToRender;
}

void Window_vulkan::teardownSwapchain()
{
    std::scoped_lock lock(GUI_globals->mutex);

    ttauri_assert(device);
    device->destroy(swapchain);
}

void Window_vulkan::buildFramebuffers()
{
    std::scoped_lock lock(GUI_globals->mutex);

    ttauri_assert(device);
    swapchainImages = device->getSwapchainImagesKHR(swapchain);
    for (auto image : swapchainImages) {
        auto imageView = device->createImageView({
            vk::ImageViewCreateFlags(),
            image,
            vk::ImageViewType::e2D,
            swapchainImageFormat.format,
            vk::ComponentMapping(),
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        });

        swapchainImageViews.push_back(imageView);

        std::array<vk::ImageView, 1> attachments = { imageView };

        auto framebuffer = device->createFramebuffer({
            vk::FramebufferCreateFlags(),
            firstRenderPass,
            numeric_cast<uint32_t>(attachments.size()),
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
    std::scoped_lock lock(GUI_globals->mutex);

    ttauri_assert(device);
    for (auto frameBuffer : swapchainFramebuffers) {
        device->destroy(frameBuffer);
    }
    swapchainFramebuffers.clear();

    for (auto imageView : swapchainImageViews) {
        device->destroy(imageView);
    }
    swapchainImageViews.clear();
}

void Window_vulkan::buildRenderPasses()
{
    std::scoped_lock lock(GUI_globals->mutex);

    std::array<vk::AttachmentDescription, 1> attachmentDescriptions = {
        vk::AttachmentDescription{
            vk::AttachmentDescriptionFlags(),
            swapchainImageFormat.format,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
            vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
            vk::ImageLayout::eUndefined, // initialLayout
            vk::ImageLayout::eColorAttachmentOptimal // finalLayout
        }
    };

    std::array<vk::AttachmentReference, 0> const inputAttachmentReferences = {};

    std::array<vk::AttachmentReference, 1> const colorAttachmentReferences = {
        vk::AttachmentReference{ 0, vk::ImageLayout::eColorAttachmentOptimal }
    };

    std::array<vk::SubpassDescription, 1> const subpassDescriptions = {
        vk::SubpassDescription{
            vk::SubpassDescriptionFlags(),
            vk::PipelineBindPoint::eGraphics,
            numeric_cast<uint32_t>(inputAttachmentReferences.size()),
            inputAttachmentReferences.data(),
            numeric_cast<uint32_t>(colorAttachmentReferences.size()),
            colorAttachmentReferences.data()
        }
    };

    std::array<vk::SubpassDependency, 1> const subpassDependency = {
        vk::SubpassDependency{
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlags(),
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
        }
    };

    vk::RenderPassCreateInfo const renderPassCreateInfo = {
        vk::RenderPassCreateFlags(),
        numeric_cast<uint32_t>(attachmentDescriptions.size()),
        attachmentDescriptions.data(),
        numeric_cast<uint32_t>(subpassDescriptions.size()),
        subpassDescriptions.data(),
        numeric_cast<uint32_t>(subpassDependency.size()),
        subpassDependency.data()
    };

    ttauri_assert(device);
    firstRenderPass = device->createRenderPass(renderPassCreateInfo);

    attachmentDescriptions.at(0).loadOp = vk::AttachmentLoadOp::eLoad;
    attachmentDescriptions.at(0).initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attachmentDescriptions.at(0).finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    followUpRenderPass = device->createRenderPass(renderPassCreateInfo);

    attachmentDescriptions.at(0).loadOp = vk::AttachmentLoadOp::eLoad;
    attachmentDescriptions.at(0).initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attachmentDescriptions.at(0).finalLayout = vk::ImageLayout::ePresentSrcKHR;
    lastRenderPass = device->createRenderPass(renderPassCreateInfo);
}

void Window_vulkan::teardownRenderPasses()
{
    std::scoped_lock lock(GUI_globals->mutex);

    ttauri_assert(device);
    device->destroy(firstRenderPass);
    device->destroy(followUpRenderPass);
    device->destroy(lastRenderPass);
}

void Window_vulkan::buildSemaphores()
{
    std::scoped_lock lock(GUI_globals->mutex);

    ttauri_assert(device);
    imageAvailableSemaphore = device->createSemaphore({});

    // This fence is used to wait for the Window and its Pipelines to be idle.
    // It should therefor be signed at the start so that when no rendering has been
    // done it is still idle.
    renderFinishedFence = device->createFence({ vk::FenceCreateFlagBits::eSignaled });
}

void Window_vulkan::teardownSemaphores()
{
    std::scoped_lock lock(GUI_globals->mutex);

    ttauri_assert(device);
    device->destroy(imageAvailableSemaphore);
    device->destroy(renderFinishedFence);
}

void Window_vulkan::teardownSurface()
{
    std::scoped_lock lock(GUI_globals->mutex);

    GUI_globals->instance().destroySurfaceKHR(intrinsic);
}

void Window_vulkan::teardownDevice()
{
    std::scoped_lock lock(GUI_globals->mutex);

    device = nullptr;
}

}
