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
#include "TTauri/GUI/DrawContext.hpp"
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
    auto lock = std::scoped_lock(guiMutex);

    Window_base::initialize();
    flatPipeline = std::make_unique<PipelineFlat::PipelineFlat>(dynamic_cast<Window &>(*this));
    boxPipeline = std::make_unique<PipelineBox::PipelineBox>(dynamic_cast<Window &>(*this));
    SDFPipeline = std::make_unique<PipelineSDF::PipelineSDF>(dynamic_cast<Window &>(*this));
    imagePipeline = std::make_unique<PipelineImage::PipelineImage>(dynamic_cast<Window &>(*this));
}

void Window_vulkan::waitIdle()
{
    auto lock = std::scoped_lock(guiMutex);

    ttauri_assert(device);
    device->waitForFences({ renderFinishedFence }, VK_TRUE, std::numeric_limits<uint64_t>::max());
    device->waitIdle();
    LOG_INFO("/waitIdle");
}

std::optional<uint32_t> Window_vulkan::acquireNextImageFromSwapchain()
{
    auto lock = std::scoped_lock(guiMutex);

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

    auto lock = std::scoped_lock(guiMutex);

    std::array<vk::Semaphore, 1> const renderFinishedSemaphores = { renderFinishedSemaphore };
    std::array<vk::SwapchainKHR, 1> const presentSwapchains = { swapchain };
    std::array<uint32_t, 1> const presentImageIndices = { frameBufferIndex };
    ttauri_assume(presentSwapchains.size() == presentImageIndices.size());

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

    // Make sure that resources are released by Vulkan by calling waitIdle.
    device->waitIdle();
}

void Window_vulkan::build()
{
    auto lock = std::scoped_lock(guiMutex);

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
    auto lock = std::scoped_lock(guiMutex);
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

void Window_vulkan::render(cpu_utc_clock::time_point displayTimePoint)
{
    // If the state is nominal (ReadyToRender) we can reduce CPU/GPU
    // usage by checking if the window was modified.
    // If the state is not-nominal then we want to get into nominal state
    // as quick as possible.
    if (state == State::ReadyToRender && renderTrigger.check(displayTimePoint) == 0) {
        return;
    }

    // While resizing lower the frame rate to reduce CPU/GPU usage.
    if (resizing && (frameCount++ % resizeFrameRateDivider) != 0) {
        return;
    }

    auto tr = trace<"win_render"_tag, "state"_tag, "frame_buffer"_tag>();
    auto lock = std::scoped_lock(guiMutex);

    // Tear down then buildup from the Vulkan objects that where invalid.
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
    ttauri_assert(device);
    device->waitForFences({ renderFinishedFence }, VK_TRUE, std::numeric_limits<uint64_t>::max());

    // Unsignal the fence so we will not modify/destroy the command buffers during rendering.
    device->resetFences({ renderFinishedFence });

    // Update the widgets before the pipelines need their vertices.
    // We unset modified before, so that modification requests are captured.
    auto drawContext = DrawContext(
        reinterpret_cast<Window &>(*this),
        flatPipeline->vertexBufferData,
        boxPipeline->vertexBufferData,
        imagePipeline->vertexBufferData,
        SDFPipeline->vertexBufferData
    );
    widget->draw(drawContext, displayTimePoint);

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
    auto lock = std::scoped_lock(guiMutex);

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    ttauri_assert(device);
    surfaceCapabilities = device->getSurfaceCapabilitiesKHR(intrinsic);

    LOG_INFO("minimumExtent=({}, {}), maximumExtent=({}, {}), currentExtent=({}, {}), osExtent=({})",
        surfaceCapabilities.minImageExtent.width, surfaceCapabilities.minImageExtent.height,
        surfaceCapabilities.maxImageExtent.width, surfaceCapabilities.maxImageExtent.height,
        surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height,
        OSWindowRectangle.extent()
    );

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
                static_cast<uint32_t>(OSWindowRectangle.width()),
                surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width
            ),
            std::clamp(
                static_cast<uint32_t>(OSWindowRectangle.height()),
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
        numeric_cast<int>(swapchainImageExtent.width) < minimumWindowExtent.x() ||
        numeric_cast<int>(swapchainImageExtent.height) < minimumWindowExtent.y()
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
        numeric_cast<int>(swapchainImageExtent.width) > maximumWindowExtent.x() ||
        numeric_cast<int>(swapchainImageExtent.height) > maximumWindowExtent.y()
        ) {
        LOG_ERROR("Window too large to draw current=({}, {}), maximum=({})",
            swapchainImageExtent.width, swapchainImageExtent.height,
            maximumWindowExtent
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

    auto lock = std::scoped_lock(guiMutex);

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

    // Create depth matching the swapchain.
    // Create atlas image
    vk::ImageCreateInfo const depthImageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        depthImageFormat,
        vk::Extent3D(swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height, 1),
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::SharingMode::eExclusive,
        0, nullptr,
        vk::ImageLayout::eUndefined
    };

    VmaAllocationCreateInfo depthAllocationCreateInfo = {};
    depthAllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    std::tie(depthImage, depthImageAllocation) = device->createImage(depthImageCreateInfo, depthAllocationCreateInfo);

    return State::ReadyToRender;
}

void Window_vulkan::teardownSwapchain()
{
    auto lock = std::scoped_lock(guiMutex);

    ttauri_assert(device);
    device->destroy(swapchain);
    device->destroyImage(depthImage, depthImageAllocation);
}

void Window_vulkan::buildFramebuffers()
{
    auto lock = std::scoped_lock(guiMutex);

    depthImageView = device->createImageView({
        vk::ImageViewCreateFlags(),
        depthImage,
        vk::ImageViewType::e2D,
        depthImageFormat,
        vk::ComponentMapping(),
        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }
    });

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

        std::array<vk::ImageView, 2> attachments = {
            imageView,
            depthImageView
        };

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

    ttauri_assume(swapchainImageViews.size() == swapchainImages.size());
    ttauri_assume(swapchainFramebuffers.size() == swapchainImages.size());
}

void Window_vulkan::teardownFramebuffers()
{
    auto lock = std::scoped_lock(guiMutex);

    ttauri_assert(device);
    for (auto frameBuffer : swapchainFramebuffers) {
        device->destroy(frameBuffer);
    }
    swapchainFramebuffers.clear();

    for (auto imageView : swapchainImageViews) {
        device->destroy(imageView);
    }
    swapchainImageViews.clear();

    device->destroy(depthImageView);
}

void Window_vulkan::buildRenderPasses()
{
    auto lock = std::scoped_lock(guiMutex);

    std::array<vk::AttachmentDescription, 2> attachmentDescriptions = {
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

        }, vk::AttachmentDescription{
            vk::AttachmentDescriptionFlags(),
            depthImageFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
            vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
            vk::ImageLayout::eUndefined, // initialLayout
            vk::ImageLayout::eDepthStencilAttachmentOptimal // finalLayout
        }
    };

    std::array<vk::AttachmentReference, 0> const inputAttachmentReferences = {};

    std::array<vk::AttachmentReference, 1> const colorAttachmentReferences = {
        vk::AttachmentReference{ 0, vk::ImageLayout::eColorAttachmentOptimal }
    };

    vk::AttachmentReference const depthAttachmentReferences = {
        1, vk::ImageLayout::eDepthStencilAttachmentOptimal
    };

    std::array<vk::SubpassDescription, 1> const subpassDescriptions = {
        vk::SubpassDescription{
            vk::SubpassDescriptionFlags(),
            vk::PipelineBindPoint::eGraphics,
            numeric_cast<uint32_t>(inputAttachmentReferences.size()),
            inputAttachmentReferences.data(),
            numeric_cast<uint32_t>(colorAttachmentReferences.size()),
            colorAttachmentReferences.data(),
            nullptr, //resolveAttachments
            &depthAttachmentReferences
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
        numeric_cast<uint32_t>(attachmentDescriptions.size()), // attachmentCount
        attachmentDescriptions.data(), // attachments
        numeric_cast<uint32_t>(subpassDescriptions.size()), // subpassCount
        subpassDescriptions.data(), // subpasses
        numeric_cast<uint32_t>(subpassDependency.size()), // dependencyCount
        subpassDependency.data() // dependencies
    };

    ttauri_assert(device);
    firstRenderPass = device->createRenderPass(renderPassCreateInfo);

    attachmentDescriptions.at(0).loadOp = vk::AttachmentLoadOp::eLoad;
    attachmentDescriptions.at(0).initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attachmentDescriptions.at(1).loadOp = vk::AttachmentLoadOp::eLoad;
    attachmentDescriptions.at(1).initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    followUpRenderPass = device->createRenderPass(renderPassCreateInfo);

    attachmentDescriptions.at(0).finalLayout = vk::ImageLayout::ePresentSrcKHR;
    attachmentDescriptions.at(1).storeOp = vk::AttachmentStoreOp::eDontCare;
    lastRenderPass = device->createRenderPass(renderPassCreateInfo);
}

void Window_vulkan::teardownRenderPasses()
{
    auto lock = std::scoped_lock(guiMutex);

    ttauri_assert(device);
    device->destroy(firstRenderPass);
    device->destroy(followUpRenderPass);
    device->destroy(lastRenderPass);
}

void Window_vulkan::buildSemaphores()
{
    auto lock = std::scoped_lock(guiMutex);

    ttauri_assert(device);
    imageAvailableSemaphore = device->createSemaphore({});

    // This fence is used to wait for the Window and its Pipelines to be idle.
    // It should therefor be signed at the start so that when no rendering has been
    // done it is still idle.
    renderFinishedFence = device->createFence({ vk::FenceCreateFlagBits::eSignaled });
}

void Window_vulkan::teardownSemaphores()
{
    auto lock = std::scoped_lock(guiMutex);

    ttauri_assert(device);
    device->destroy(imageAvailableSemaphore);
    device->destroy(renderFinishedFence);
}

void Window_vulkan::teardownSurface()
{
    auto lock = std::scoped_lock(guiMutex);

    guiSystem->destroySurfaceKHR(intrinsic);
}

void Window_vulkan::teardownDevice()
{
    auto lock = std::scoped_lock(guiMutex);

    device = nullptr;
}

}
