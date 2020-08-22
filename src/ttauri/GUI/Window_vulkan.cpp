// Copyright 2019 Pokitec
// All rights reserved.

#include "Window_vulkan.hpp"
#include "Window.hpp"
#include "GUISystem.hpp"
#include "GUIDevice.hpp"
#include "PipelineFlat.hpp"
#include "PipelineBox.hpp"
#include "PipelineImage.hpp"
#include "PipelineSDF.hpp"
#include "PipelineToneMapper.hpp"
#include "DrawContext.hpp"
#include "../widgets/WindowWidget.hpp"
#include "../trace.hpp"
#include "../Application.hpp"
#include <vector>

namespace tt {

using namespace std;

Window_vulkan::Window_vulkan(WindowDelegate *delegate, Label &&title) :
    Window_base(delegate, std::move(title)), nrSwapchainImages(0), swapchainImageFormat()
{
}

Window_vulkan::~Window_vulkan()
{
}

void Window_vulkan::initialize()
{
    ttlet lock = std::scoped_lock(mutex);

    Window_base::initialize();
    flatPipeline = std::make_unique<PipelineFlat::PipelineFlat>(dynamic_cast<Window &>(*this));
    boxPipeline = std::make_unique<PipelineBox::PipelineBox>(dynamic_cast<Window &>(*this));
    imagePipeline = std::make_unique<PipelineImage::PipelineImage>(dynamic_cast<Window &>(*this));
    SDFPipeline = std::make_unique<PipelineSDF::PipelineSDF>(dynamic_cast<Window &>(*this));
    toneMapperPipeline = std::make_unique<PipelineToneMapper::PipelineToneMapper>(dynamic_cast<Window &>(*this));
}

void Window_vulkan::waitIdle()
{
    ttlet lock = std::scoped_lock(mutex);

    tt_assert(device);
    if (renderFinishedFence) {
        device->waitForFences({ renderFinishedFence }, VK_TRUE, std::numeric_limits<uint64_t>::max());
    }
    device->waitIdle();
    LOG_INFO("/waitIdle");
}

std::optional<uint32_t> Window_vulkan::acquireNextImageFromSwapchain()
{
    ttlet lock = std::scoped_lock(mutex);

    // swap chain, fence & imageAvailableSemaphore must be externally synchronized.
    uint32_t frameBufferIndex = 0;
    //LOG_DEBUG("acquireNextImage '{}'", title);
    tt_assert(device);
    ttlet result = device->acquireNextImageKHR(swapchain, 0, imageAvailableSemaphore, vk::Fence(), &frameBufferIndex);
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
            .set<vk_result_tag>(to_string(result))
        );
    }
}

void Window_vulkan::presentImageToQueue(uint32_t frameBufferIndex, vk::Semaphore semaphore)
{
    auto lock = std::scoped_lock(mutex);

    tt_assume(device);

    std::array<vk::Semaphore, 1> const renderFinishedSemaphores = { semaphore };
    std::array<vk::SwapchainKHR, 1> const presentSwapchains = { swapchain };
    std::array<uint32_t, 1> const presentImageIndices = { frameBufferIndex };
    tt_assume(presentSwapchains.size() == presentImageIndices.size());

    try {
        //LOG_DEBUG("presentQueue {}", presentImageIndices.at(0));
        ttlet result = device->presentQueue.presentKHR({
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
                .set<vk_result_tag>(to_string(result))
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
}

void Window_vulkan::build()
{
    auto lock = std::scoped_lock(mutex);

    if (state == State::NoDevice) {
        if (device) {
            flatPipeline->buildForNewDevice(device);
            boxPipeline->buildForNewDevice(device);
            imagePipeline->buildForNewDevice(device);
            SDFPipeline->buildForNewDevice(device);
            toneMapperPipeline->buildForNewDevice(device);
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
        toneMapperPipeline->buildForNewSurface();
        state = State::NoSwapchain;
    }

    if (state == State::NoSwapchain) {
        if (!readSurfaceExtent()) {
            // Minimized window, can not build a new swap chain.
            state = State::NoSwapchain;
            return;
        }

        ttlet s = buildSwapchain();
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
        buildRenderPasses(); // Render-pass requires the swapchain/color/depth image-format.
        buildFramebuffers(); // Framebuffer required render passes.
        buildCommandBuffers();
        buildSemaphores();
        flatPipeline->buildForNewSwapchain(renderPass, 0, swapchainImageExtent);
        boxPipeline->buildForNewSwapchain(renderPass, 1, swapchainImageExtent);
        imagePipeline->buildForNewSwapchain(renderPass, 2, swapchainImageExtent);
        SDFPipeline->buildForNewSwapchain(renderPass, 3, swapchainImageExtent);
        toneMapperPipeline->buildForNewSwapchain(renderPass, 4, swapchainImageExtent);

        windowChangedSize({
            numeric_cast<float>(swapchainImageExtent.width),
            numeric_cast<float>(swapchainImageExtent.height)
        });
        state = State::ReadyToRender;
    }
}

void Window_vulkan::teardown()
{
    auto lock = std::scoped_lock(mutex);

    auto nextState = state;

    if (state >= State::SwapchainLost) {
        LOG_INFO("Tearing down because the window lost the swapchain.");
        waitIdle();
        toneMapperPipeline->teardownForSwapchainLost();
        SDFPipeline->teardownForSwapchainLost();
        imagePipeline->teardownForSwapchainLost();
        boxPipeline->teardownForSwapchainLost();
        flatPipeline->teardownForSwapchainLost();
        teardownSemaphores();
        teardownCommandBuffers();
        teardownFramebuffers();
        teardownRenderPasses();
        teardownSwapchain();
        nextState = State::NoSwapchain;

        if (state >= State::SurfaceLost) {
            LOG_INFO("Tearing down because the window lost the drawable surface.");
            toneMapperPipeline->teardownForSurfaceLost();
            SDFPipeline->teardownForSurfaceLost();
            imagePipeline->teardownForSurfaceLost();
            boxPipeline->teardownForSurfaceLost();
            flatPipeline->teardownForSurfaceLost();
            teardownSurface();
            nextState = State::NoSurface;

            if (state >= State::DeviceLost) {
                LOG_INFO("Tearing down because the window lost the vulkan device.");

                toneMapperPipeline->teardownForDeviceLost();
                SDFPipeline->teardownForDeviceLost();
                imagePipeline->teardownForDeviceLost();
                boxPipeline->teardownForDeviceLost();
                flatPipeline->teardownForDeviceLost();
                teardownDevice();
                nextState = State::NoDevice;

                if (state >= State::WindowLost) {
                    LOG_INFO("Tearing down because the window doesn't exist anymore.");

                    toneMapperPipeline->teardownForWindowLost();
                    SDFPipeline->teardownForWindowLost();
                    imagePipeline->teardownForWindowLost();
                    boxPipeline->teardownForWindowLost();
                    flatPipeline->teardownForWindowLost();

                    delegate->closingWindow(*static_cast<Window *>(this));
                    nextState = State::NoWindow;
                }
            }
        }
    }
    state = nextState;
}

void Window_vulkan::render(hires_utc_clock::time_point displayTimePoint)
{
    auto lock = std::scoped_lock(mutex);

    // Tear down then buildup from the Vulkan objects that where invalid.
    teardown();
    build();

    // Bail out when the window is not yet ready to be rendered.
    if (state != State::ReadyToRender) {
        return;
    }

    {
        ttlet widget_lock = std::scoped_lock(widget->mutex);
        auto needLayout = widget->updateConstraints() >= WidgetUpdateResult::Children;
        needLayout |= requestLayout.exchange(false, std::memory_order::memory_order_relaxed);
        if (needLayout) {
            layoutWindow();
        }

        // Make sure the widget's layout is updated before draw, but after window resize.
        auto needRedraw = widget->updateLayout(displayTimePoint, needLayout) >= WidgetUpdateResult::Children;
        needRedraw |= requestRedraw.exchange(false, std::memory_order::memory_order_relaxed);

        if (!needRedraw) {
            return;
        }
    }

    struct window_render_tag {};
    struct frame_buffer_index_tag {};
    auto tr = trace<window_render_tag, frame_buffer_index_tag>();

    ttlet optionalFrameBufferIndex = acquireNextImageFromSwapchain();
    if (!optionalFrameBufferIndex) {
        // No image is ready to be rendered, yet, possibly because our vertical sync function
        // is not working correctly.
        return;
    }
    ttlet frameBufferIndex = *optionalFrameBufferIndex;
    ttlet frameBuffer = swapchainFramebuffers.at(frameBufferIndex);

    tr.set<frame_buffer_index_tag>(frameBufferIndex);

    // Wait until previous rendering has finished, before the next rendering.
    tt_assert(device);
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
    drawContext.transform = drawContext.transform * mat::T{0.5, 0.5};

    {
        ttlet widget_lock = std::scoped_lock(widget->mutex);
        widget->draw(drawContext, displayTimePoint);
    }

    fillCommandBuffer(frameBuffer);
    submitCommandBuffer();

    // Signal the fence when all rendering has finished on the graphics queue.
    // When the fence is signaled we can modify/destroy the command buffers.
    device->graphicsQueue.submit(0, nullptr, renderFinishedFence);

    presentImageToQueue(frameBufferIndex, renderFinishedSemaphore);

    // Do an early tear down of invalid vulkan objects.
    teardown();
}

void Window_vulkan::fillCommandBuffer(vk::Framebuffer frameBuffer)
{
    struct fill_command_buffer_tag {};
    auto t = trace<fill_command_buffer_tag>{};

    commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    ttlet colorClearValue = vk::ClearColorValue{
        static_cast<std::array<float,4>>(theme->fillColor(0))
    };
    ttlet depthClearValue = vk::ClearDepthStencilValue{0.0, 0};
    ttlet clearValues = std::array{
        vk::ClearValue{ colorClearValue },
        vk::ClearValue{ colorClearValue },
        vk::ClearValue{ depthClearValue }
    };

    ttlet renderArea = vk::Rect2D{vk::Offset2D{ 0, 0 }, swapchainImageExtent};

    commandBuffer.beginRenderPass({
        renderPass, 
        frameBuffer,
        renderArea, 
        numeric_cast<uint32_t>(clearValues.size()),
        clearValues.data()
        }, vk::SubpassContents::eInline
    );

    flatPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    boxPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    imagePipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    SDFPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    toneMapperPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.endRenderPass();
    commandBuffer.end();
}

void Window_vulkan::submitCommandBuffer()
{
    ttlet waitSemaphores = std::array{
        imageAvailableSemaphore
    };

    ttlet waitStages = std::array{
        vk::PipelineStageFlags{vk::PipelineStageFlagBits::eColorAttachmentOutput}
    };

    tt_assume(waitSemaphores.size() == waitStages.size());

    ttlet signalSemaphores = std::array{ renderFinishedSemaphore };
    ttlet commandBuffersToSubmit = std::array{ commandBuffer };

    ttlet submitInfo = std::array{
        vk::SubmitInfo{
            numeric_cast<uint32_t>(waitSemaphores.size()), waitSemaphores.data(), waitStages.data(),
            numeric_cast<uint32_t>(commandBuffersToSubmit.size()), commandBuffersToSubmit.data(),
            numeric_cast<uint32_t>(signalSemaphores.size()), signalSemaphores.data()
        }
    };

    tt_assume(device != nullptr);
    device->graphicsQueue.submit(submitInfo, vk::Fence());
}

std::tuple<uint32_t, vk::Extent2D> Window_vulkan::getImageCountAndExtent()
{
    auto lock = std::scoped_lock(mutex);

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    tt_assert(device);
    surfaceCapabilities = device->getSurfaceCapabilitiesKHR(intrinsic);

    LOG_INFO("minimumExtent=({}, {}), maximumExtent=({}, {}), currentExtent=({}, {})",
        surfaceCapabilities.minImageExtent.width, surfaceCapabilities.minImageExtent.height,
        surfaceCapabilities.maxImageExtent.width, surfaceCapabilities.maxImageExtent.height,
        surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height
    );

    ttlet currentExtentSet =
        (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) &&
        (surfaceCapabilities.currentExtent.height != std::numeric_limits<uint32_t>::max());

    if (!currentExtentSet) {
        // XXX On wayland, the window size is based on the size of the swapchain, so I need
        // to build a way of manual resizing the window outside of the operating system.
        LOG_FATAL("getSurfaceCapabilitiesKHR() does not supply currentExtent");
    }

    ttlet minImageCount = surfaceCapabilities.minImageCount;
    ttlet maxImageCount = surfaceCapabilities.maxImageCount ? surfaceCapabilities.maxImageCount : 10;
    ttlet imageCount = std::clamp(defaultNumberOfSwapchainImages, minImageCount, maxImageCount);

    return { imageCount, surfaceCapabilities.currentExtent };
}

bool Window_vulkan::readSurfaceExtent()
{
    try {
        std::tie(nrSwapchainImages, swapchainImageExtent) = getImageCountAndExtent();

    } catch (const vk::SurfaceLostKHRError&) {
        state = State::SurfaceLost;
        return false;
    }

    tt_assume(widget);
    ttlet widget_lock = std::scoped_lock(widget->mutex);

    if (
        numeric_cast<int>(swapchainImageExtent.width) < widget->minimumExtent().width() ||
        numeric_cast<int>(swapchainImageExtent.height) < widget->minimumExtent().height()
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
        numeric_cast<int>(swapchainImageExtent.width) > widget->maximumExtent().width() ||
        numeric_cast<int>(swapchainImageExtent.height) > widget->maximumExtent().height()
        ) {
        LOG_ERROR("Window too large to draw current=({}, {}), maximum=({})",
            swapchainImageExtent.width, swapchainImageExtent.height,
            widget->maximumExtent()
        );
        return false;
    }

    return true;
}

bool Window_vulkan::checkSurfaceExtent()
{
    try {
        ttlet [nrImages, extent] = getImageCountAndExtent();
        return (nrImages == static_cast<uint32_t>(nrSwapchainImages)) && (extent == swapchainImageExtent);

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

    tt_assert(device);
    return device->score(intrinsic) > 0;
}

Window_base::State Window_vulkan::buildSwapchain()
{
    auto lock = std::scoped_lock(mutex);

    tt_assume(device);

    LOG_INFO("Building swap chain");

    ttlet sharingMode = device->graphicsQueueFamilyIndex == device->presentQueueFamilyIndex ?
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
            .set<vk_result_tag>(to_string(result))
        );
    }


    LOG_INFO("Finished building swap chain");
    LOG_INFO(" - extent=({}, {})", swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height);
    LOG_INFO(" - colorSpace={}, format={}", vk::to_string(swapchainCreateInfo.imageColorSpace), vk::to_string(swapchainCreateInfo.imageFormat));
    LOG_INFO(" - presentMode={}, imageCount={}", vk::to_string(swapchainCreateInfo.presentMode), swapchainCreateInfo.minImageCount);

    // Create depth matching the swapchain.
    vk::ImageCreateInfo const depthImageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        depthImageFormat,
        vk::Extent3D(swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height, 1),
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | device->transientImageUsageFlags,
        vk::SharingMode::eExclusive,
        0, nullptr,
        vk::ImageLayout::eUndefined
    };

    VmaAllocationCreateInfo depthAllocationCreateInfo = {};
    depthAllocationCreateInfo.usage = device->lazyMemoryUsage;
    std::tie(depthImage, depthImageAllocation) = device->createImage(depthImageCreateInfo, depthAllocationCreateInfo);

    // Create color image matching the swapchain.
    vk::ImageCreateInfo const colorImageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        colorImageFormat,
        vk::Extent3D(swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height, 1),
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | device->transientImageUsageFlags,
        vk::SharingMode::eExclusive,
        0, nullptr,
        vk::ImageLayout::eUndefined
    };
    
    VmaAllocationCreateInfo colorAllocationCreateInfo = {};
    colorAllocationCreateInfo.usage = device->lazyMemoryUsage;
    std::tie(colorImage, colorImageAllocation) = device->createImage(colorImageCreateInfo, colorAllocationCreateInfo);
    
    return State::ReadyToRender;
}

void Window_vulkan::teardownSwapchain()
{
    auto lock = std::scoped_lock(mutex);

    tt_assert(device);
    device->destroy(swapchain);
    device->destroyImage(depthImage, depthImageAllocation);
    device->destroyImage(colorImage, colorImageAllocation);
}

void Window_vulkan::buildFramebuffers()
{
    auto lock = std::scoped_lock(mutex);

    depthImageView = device->createImageView({
        vk::ImageViewCreateFlags(),
        depthImage,
        vk::ImageViewType::e2D,
        depthImageFormat,
        vk::ComponentMapping(),
        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }
    });

    colorImageView = device->createImageView({
        vk::ImageViewCreateFlags(),
        colorImage,
        vk::ImageViewType::e2D,
        colorImageFormat,
        vk::ComponentMapping(),
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    });

    colorDescriptorImageInfo = {
        vk::Sampler(),
        colorImageView,
        vk::ImageLayout::eShaderReadOnlyOptimal
    };

    tt_assert(device);
    swapchainImages = device->getSwapchainImagesKHR(swapchain);
    for (auto image : swapchainImages) {
        ttlet swapchainImageView = device->createImageView({
            vk::ImageViewCreateFlags(),
            image,
            vk::ImageViewType::e2D,
            swapchainImageFormat.format,
            vk::ComponentMapping(),
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        });

        swapchainImageViews.push_back(swapchainImageView);

        ttlet attachments = std::array{
            swapchainImageView,
            colorImageView,
            depthImageView
        };

        ttlet framebuffer = device->createFramebuffer({
            vk::FramebufferCreateFlags(),
            renderPass,
            numeric_cast<uint32_t>(attachments.size()),
            attachments.data(),
            swapchainImageExtent.width,
            swapchainImageExtent.height,
            1 // layers
        });
        swapchainFramebuffers.push_back(framebuffer);
    }

    tt_assume(swapchainImageViews.size() == swapchainImages.size());
    tt_assume(swapchainFramebuffers.size() == swapchainImages.size());
}

void Window_vulkan::teardownFramebuffers()
{
    auto lock = std::scoped_lock(mutex);

    tt_assert(device);
    for (auto frameBuffer : swapchainFramebuffers) {
        device->destroy(frameBuffer);
    }
    swapchainFramebuffers.clear();

    for (auto imageView : swapchainImageViews) {
        device->destroy(imageView);
    }
    swapchainImageViews.clear();

    device->destroy(depthImageView);
    device->destroy(colorImageView);
}

void Window_vulkan::buildRenderPasses()
{
    auto lock = std::scoped_lock(mutex);

    ttlet attachmentDescriptions = std::array{
        vk::AttachmentDescription{ // Swapchain attachment.
            vk::AttachmentDescriptionFlags(),
            swapchainImageFormat.format,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
            vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
            vk::ImageLayout::eUndefined, // initialLayout
            vk::ImageLayout::ePresentSrcKHR // finalLayout

        }, vk::AttachmentDescription{ // Color attachment
            vk::AttachmentDescriptionFlags(),
            colorImageFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
            vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
            vk::ImageLayout::eUndefined, // initialLayout
            vk::ImageLayout::eColorAttachmentOptimal // finalLayout

        }, vk::AttachmentDescription{ // Depth attachment
            vk::AttachmentDescriptionFlags(),
            depthImageFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
            vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
            vk::ImageLayout::eUndefined, // initialLayout
            vk::ImageLayout::eDepthStencilAttachmentOptimal // finalLayout
        }
    };

    ttlet colorAttachmentReferences = std::array{
        vk::AttachmentReference{ 1, vk::ImageLayout::eColorAttachmentOptimal }
    };

    ttlet colorInputAttachmentReferences = std::array{
        vk::AttachmentReference{ 1, vk::ImageLayout::eShaderReadOnlyOptimal }
    };

    ttlet swapchainAttachmentReferences = std::array{
        vk::AttachmentReference{ 0, vk::ImageLayout::eColorAttachmentOptimal }
    };

    ttlet depthAttachmentReference = vk::AttachmentReference{
        2, vk::ImageLayout::eDepthStencilAttachmentOptimal
    };

    ttlet subpassDescriptions = std::array{
        vk::SubpassDescription{ // Subpass 0
            vk::SubpassDescriptionFlags(),
            vk::PipelineBindPoint::eGraphics,
            0, // inputAttchmentReferencesCount
            nullptr, // inputAttachmentReferences
            numeric_cast<uint32_t>(colorAttachmentReferences.size()),
            colorAttachmentReferences.data(),
            nullptr, //resolveAttachments
            &depthAttachmentReference

        }, vk::SubpassDescription{ // Subpass 1
            vk::SubpassDescriptionFlags(),
            vk::PipelineBindPoint::eGraphics,
            0, // inputAttchmentReferencesCount
            nullptr, // inputAttachmentReferences
            numeric_cast<uint32_t>(colorAttachmentReferences.size()),
            colorAttachmentReferences.data(),
            nullptr, //resolveAttachments
            &depthAttachmentReference

        }, vk::SubpassDescription{ // Subpass 2
            vk::SubpassDescriptionFlags(),
            vk::PipelineBindPoint::eGraphics,
            0, // inputAttchmentReferencesCount
            nullptr, // inputAttachmentReferences
            numeric_cast<uint32_t>(colorAttachmentReferences.size()),
            colorAttachmentReferences.data(),
            nullptr, //resolveAttachments
            &depthAttachmentReference

        }, vk::SubpassDescription{ // Subpass 3
            vk::SubpassDescriptionFlags(),
            vk::PipelineBindPoint::eGraphics,
            numeric_cast<uint32_t>(colorInputAttachmentReferences.size()),
            colorInputAttachmentReferences.data(),
            numeric_cast<uint32_t>(colorAttachmentReferences.size()),
            colorAttachmentReferences.data(),
            nullptr, // resolveAttachments
            &depthAttachmentReference

        }, vk::SubpassDescription{ // Subpass 4 tone-mapper
            vk::SubpassDescriptionFlags(),
            vk::PipelineBindPoint::eGraphics,
            numeric_cast<uint32_t>(colorInputAttachmentReferences.size()),
            colorInputAttachmentReferences.data(),
            numeric_cast<uint32_t>(swapchainAttachmentReferences.size()),
            swapchainAttachmentReferences.data(),
            nullptr,
            nullptr
        }
    };

    ttlet subpassDependency = std::array{
        vk::SubpassDependency{
            VK_SUBPASS_EXTERNAL, 0,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eMemoryRead,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::DependencyFlagBits::eByRegion
        },
        // Subpass 0: Render single color polygons to color+depth attachment.
        vk::SubpassDependency{
            0, 1,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentRead,
            vk::DependencyFlagBits::eByRegion
        },
        // Subpass 1: Render shaded polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            1, 2,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentRead,
            vk::DependencyFlagBits::eByRegion
        },
        // Subpass 2: Render texture mapped polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            2, 3,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eShaderRead,
            vk::DependencyFlagBits::eByRegion
        },
        // Subpass 3: Render SDF-texture mapped polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            3, 4,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eShaderRead,
            vk::DependencyFlagBits::eByRegion
        },
        // Subpass 4: Tone mapping color to swapchain.
        vk::SubpassDependency{
            4, VK_SUBPASS_EXTERNAL,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eMemoryRead,
            vk::DependencyFlagBits::eByRegion
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

    tt_assert(device);
    renderPass = device->createRenderPass(renderPassCreateInfo);
}

void Window_vulkan::teardownRenderPasses()
{
    auto lock = std::scoped_lock(mutex);

    tt_assert(device);
    device->destroy(renderPass);
}

void Window_vulkan::buildSemaphores()
{
    auto lock = std::scoped_lock(mutex);

    tt_assert(device);
    imageAvailableSemaphore = device->createSemaphore({});
    renderFinishedSemaphore = device->createSemaphore({});

    // This fence is used to wait for the Window and its Pipelines to be idle.
    // It should therefor be signed at the start so that when no rendering has been
    // done it is still idle.
    renderFinishedFence = device->createFence({ vk::FenceCreateFlagBits::eSignaled });
}

void Window_vulkan::teardownSemaphores()
{
    auto lock = std::scoped_lock(mutex);

    tt_assert(device);
    device->destroy(renderFinishedSemaphore);
    device->destroy(imageAvailableSemaphore);
    device->destroy(renderFinishedFence);
}

void Window_vulkan::buildCommandBuffers()
{
    auto lock = std::scoped_lock(mutex);

    tt_assume(device != nullptr);

    ttlet commandBuffers = device->allocateCommandBuffers({
        device->graphicsCommandPool, 
        vk::CommandBufferLevel::ePrimary, 
        1
    });

    commandBuffer = commandBuffers.at(0);
}

void Window_vulkan::teardownCommandBuffers()
{
    auto lock = std::scoped_lock(mutex);
    ttlet commandBuffers = std::vector<vk::CommandBuffer>{commandBuffer};

    tt_assume(device != nullptr);
    device->freeCommandBuffers(device->graphicsCommandPool, commandBuffers);
}

void Window_vulkan::teardownSurface()
{
    auto lock = std::scoped_lock(mutex);

    application->gui->destroySurfaceKHR(intrinsic);
}

void Window_vulkan::teardownDevice()
{
    auto lock = std::scoped_lock(mutex);

    device = nullptr;
}

}
