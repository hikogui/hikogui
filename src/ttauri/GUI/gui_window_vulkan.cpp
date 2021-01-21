// Copyright 2019 Pokitec
// All rights reserved.

#include "gui_window_vulkan.hpp"
#include "gui_system_vulkan.hpp"
#include "gui_device_vulkan.hpp"
#include "PipelineFlat.hpp"
#include "PipelineBox.hpp"
#include "PipelineImage.hpp"
#include "PipelineSDF.hpp"
#include "PipelineToneMapper.hpp"
#include "draw_context.hpp"
#include "../widgets/WindowWidget.hpp"
#include "../trace.hpp"
#include "../application.hpp"
#include "../cast.hpp"
#include <vector>

namespace tt {

using namespace std;

gui_window_vulkan::gui_window_vulkan(gui_system &system, std::weak_ptr<gui_window_delegate> const &delegate, label const &title) :
    gui_window(system, delegate, title), nrSwapchainImages(0), swapchainImageFormat()
{
}

gui_window_vulkan::~gui_window_vulkan() {}

gui_device_vulkan &gui_window_vulkan::vulkan_device() const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    tt_axiom(_device != nullptr);
    return narrow_cast<gui_device_vulkan &>(*_device);
}

void gui_window_vulkan::init()
{
    // This function is called just after construction in single threaded mode,
    // and therefor should not have a lock on the window.
    tt_assert(is_main_thread(), "createWindow should be called from the main thread.");
    tt_axiom(gui_system_mutex.recurse_lock_count() == 0);

    gui_window::init();
    flatPipeline = std::make_unique<PipelineFlat::PipelineFlat>(*this);
    boxPipeline = std::make_unique<PipelineBox::PipelineBox>(*this);
    imagePipeline = std::make_unique<PipelineImage::PipelineImage>(*this);
    SDFPipeline = std::make_unique<PipelineSDF::PipelineSDF>(*this);
    toneMapperPipeline = std::make_unique<PipelineToneMapper::PipelineToneMapper>(*this);
}

void gui_window_vulkan::waitIdle()
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    tt_assert(_device);
    if (renderFinishedFence) {
        vulkan_device().waitForFences({renderFinishedFence}, VK_TRUE, std::numeric_limits<uint64_t>::max());
    }
    vulkan_device().waitIdle();
    LOG_INFO("/waitIdle");
}

std::optional<uint32_t> gui_window_vulkan::acquireNextImageFromSwapchain()
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    // swap chain, fence & imageAvailableSemaphore must be externally synchronized.
    uint32_t frameBufferIndex = 0;
    // LOG_DEBUG("acquireNextImage '{}'", title);

    ttlet result = vulkan_device().acquireNextImageKHR(swapchain, 0, imageAvailableSemaphore, vk::Fence(), &frameBufferIndex);
    // LOG_DEBUG("acquireNextImage {}", frameBufferIndex);

    switch (result) {
    case vk::Result::eSuccess: return {frameBufferIndex};

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

    default: tt_error_info().set<vk_result_tag>(result); throw gui_error("Unknown result from acquireNextImageKHR()");
    }
}

void gui_window_vulkan::presentImageToQueue(uint32_t frameBufferIndex, vk::Semaphore semaphore)
{
    auto lock = std::scoped_lock(gui_system_mutex);

    tt_axiom(_device);

    std::array<vk::Semaphore, 1> const renderFinishedSemaphores = {semaphore};
    std::array<vk::SwapchainKHR, 1> const presentSwapchains = {swapchain};
    std::array<uint32_t, 1> const presentImageIndices = {frameBufferIndex};
    tt_axiom(presentSwapchains.size() == presentImageIndices.size());

    try {
        // LOG_DEBUG("presentQueue {}", presentImageIndices.at(0));
        ttlet result = vulkan_device().presentQueue.presentKHR(
            {narrow_cast<uint32_t>(renderFinishedSemaphores.size()),
             renderFinishedSemaphores.data(),
             narrow_cast<uint32_t>(presentSwapchains.size()),
             presentSwapchains.data(),
             presentImageIndices.data()});

        switch (result) {
        case vk::Result::eSuccess: return;

        case vk::Result::eSuboptimalKHR:
            LOG_INFO("presentKHR() eSuboptimalKHR");
            state = State::SwapchainLost;
            return;

        default: tt_error_info().set<vk_result_tag>(result); throw gui_error("Unknown result from presentKHR()");
        }

    } catch (vk::OutOfDateKHRError const &) {
        LOG_INFO("presentKHR() eErrorOutOfDateKHR");
        state = State::SwapchainLost;
        return;

    } catch (vk::SurfaceLostKHRError const &) {
        LOG_INFO("presentKHR() eErrorSurfaceLostKHR");
        state = State::SurfaceLost;
        return;
    }
}

void gui_window_vulkan::build()
{
    auto lock = std::scoped_lock(gui_system_mutex);

    if (state == State::NoDevice) {
        if (_device) {
            flatPipeline->buildForNewDevice();
            boxPipeline->buildForNewDevice();
            imagePipeline->buildForNewDevice();
            SDFPipeline->buildForNewDevice();
            toneMapperPipeline->buildForNewDevice();
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

        window_changed_size({narrow_cast<float>(swapchainImageExtent.width), narrow_cast<float>(swapchainImageExtent.height)});
        state = State::ReadyToRender;
    }
}

void gui_window_vulkan::teardown()
{
    auto lock = std::scoped_lock(gui_system_mutex);

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

                    if (auto delegate_ = delegate.lock()) {
                        delegate_->deinit(*this);
                    }
                    nextState = State::NoWindow;
                }
            }
        }
    }
    state = nextState;
}

void gui_window_vulkan::render(hires_utc_clock::time_point displayTimePoint)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    // Tear down then buildup from the Vulkan objects that where invalid.
    teardown();
    build();

    // Bail out when the window is not yet ready to be rendered.
    if (state != State::ReadyToRender) {
        return;
    }

    // All widgets need constrains recalculated on these window-wide events.
    // Like theme or language changes.
    ttlet need_reconstrain = std::exchange(_request_setting_change, false);

    // Update the size constraints of the WindowWidget and it children.
    ttlet constraints_have_changed = widget->update_constraints(displayTimePoint, need_reconstrain);

    // Check if the window size matches the preferred size of the WindowWidget.
    // If not ask the operating system to change the size of the window, which is
    // done asynchronously.
    //
    // We need to continue drawing into the incorrectly sized window, otherwise
    // Vulkan will not detect the change of drawing surface's size.
    //
    // Make sure the widget does have its window rectangle match the constraints, otherwise
    // the logic for layout and drawing becomes complicated.
    ttlet preferred_size = widget->preferred_size();
    if (requestResize.exchange(false) || current_window_extent << preferred_size) {
        set_window_size(current_window_extent = preferred_size.minimum());
    } else if (current_window_extent >> preferred_size) {
        set_window_size(current_window_extent = preferred_size.maximum());
    }
    widget->set_layout_parameters(aarect{current_window_extent}, aarect{current_window_extent});

    // When a window message was received, such as a resize, redraw, language-change; the requestLayout is set to true.
    ttlet need_layout = requestLayout.exchange(false, std::memory_order::memory_order_relaxed) || constraints_have_changed;

    // Make sure the widget's layout is updated before draw, but after window resize.
    widget->update_layout(displayTimePoint, need_layout);

    if (!static_cast<bool>(_request_redraw_rectangle)) {
        return;
    }

    struct window_render_tag {
    };
    struct frame_buffer_index_tag {
    };
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
    vulkan_device().waitForFences({renderFinishedFence}, VK_TRUE, std::numeric_limits<uint64_t>::max());

    // Unsignal the fence so we will not modify/destroy the command buffers during rendering.
    vulkan_device().resetFences({renderFinishedFence});

    // Record which part of the image will be redrawn on the current swapchain image.
    swapchainRedrawRectangle.at(frameBufferIndex) = _request_redraw_rectangle;

    // Calculate the scissor rectangle, from the combined redraws of the complete swapchain.
    // We need to do this so that old redraws are also executed in the current swapchain image.
    ttlet scissor_rectangle = ceil(std::accumulate(
        swapchainRedrawRectangle.cbegin(), swapchainRedrawRectangle.cend(), aarect{}, [](ttlet &sum, ttlet &item) {
            return sum | item;
        }));

    // Update the widgets before the pipelines need their vertices.
    // We unset modified before, so that modification requests are captured.
    auto drawContext = draw_context(
        *this,
        scissor_rectangle,
        flatPipeline->vertexBufferData,
        boxPipeline->vertexBufferData,
        imagePipeline->vertexBufferData,
        SDFPipeline->vertexBufferData);
    drawContext.transform = drawContext.transform * mat::T{0.5, 0.5};

    _request_redraw_rectangle = aarect{};
    widget->draw(drawContext, displayTimePoint);

    fillCommandBuffer(frameBuffer, scissor_rectangle);
    submitCommandBuffer();

    // Signal the fence when all rendering has finished on the graphics queue.
    // When the fence is signaled we can modify/destroy the command buffers.
    vulkan_device().graphicsQueue.submit(0, nullptr, renderFinishedFence);

    presentImageToQueue(frameBufferIndex, renderFinishedSemaphore);

    // Do an early tear down of invalid vulkan objects.
    teardown();
}

void gui_window_vulkan::fillCommandBuffer(vk::Framebuffer frameBuffer, aarect scissor_rectangle)
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    struct fill_command_buffer_tag {
    };
    auto t = trace<fill_command_buffer_tag>{};

    commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    ttlet colorClearValue = vk::ClearColorValue{static_cast<std::array<float, 4>>(widget->backgroundColor())};
    ttlet depthClearValue = vk::ClearDepthStencilValue{0.0, 0};
    ttlet clearValues =
        std::array{vk::ClearValue{colorClearValue}, vk::ClearValue{colorClearValue}, vk::ClearValue{depthClearValue}};

    // Clamp the scissor rectangle to the size of the window.
    scissor_rectangle = intersect(scissor_rectangle, aarect{0.0f, 0.0f, swapchainImageExtent.width, swapchainImageExtent.height});
    scissor_rectangle = ceil(scissor_rectangle);

    ttlet scissors = std::array{vk::Rect2D{
        vk::Offset2D(
            narrow_cast<uint32_t>(scissor_rectangle.x()),
            narrow_cast<uint32_t>(swapchainImageExtent.height - scissor_rectangle.y() - scissor_rectangle.height())),
        vk::Extent2D(narrow_cast<uint32_t>(scissor_rectangle.width()), narrow_cast<uint32_t>(scissor_rectangle.height()))}};

    // The scissor and render area makes sure that the frame buffer is not modified where we are not drawing the widgets.
    commandBuffer.setScissor(0, scissors);

    ttlet renderArea = scissors.at(0);

    // ttlet renderArea = vk::Rect2D{
    //    vk::Offset2D(0, 0), vk::Extent2D(swapchainImageExtent.width, swapchainImageExtent.height)};

    commandBuffer.beginRenderPass(
        {renderPass, frameBuffer, renderArea, narrow_cast<uint32_t>(clearValues.size()), clearValues.data()},
        vk::SubpassContents::eInline);

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

void gui_window_vulkan::submitCommandBuffer()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    ttlet waitSemaphores = std::array{imageAvailableSemaphore};

    ttlet waitStages = std::array{vk::PipelineStageFlags{vk::PipelineStageFlagBits::eColorAttachmentOutput}};

    tt_axiom(waitSemaphores.size() == waitStages.size());

    ttlet signalSemaphores = std::array{renderFinishedSemaphore};
    ttlet commandBuffersToSubmit = std::array{commandBuffer};

    ttlet submitInfo = std::array{vk::SubmitInfo{
        narrow_cast<uint32_t>(waitSemaphores.size()),
        waitSemaphores.data(),
        waitStages.data(),
        narrow_cast<uint32_t>(commandBuffersToSubmit.size()),
        commandBuffersToSubmit.data(),
        narrow_cast<uint32_t>(signalSemaphores.size()),
        signalSemaphores.data()}};

    vulkan_device().graphicsQueue.submit(submitInfo, vk::Fence());
}

std::tuple<uint32_t, vk::Extent2D> gui_window_vulkan::getImageCountAndExtent()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    surfaceCapabilities = vulkan_device().getSurfaceCapabilitiesKHR(intrinsic);

    LOG_INFO(
        "minimumExtent=({}, {}), maximumExtent=({}, {}), currentExtent=({}, {})",
        surfaceCapabilities.minImageExtent.width,
        surfaceCapabilities.minImageExtent.height,
        surfaceCapabilities.maxImageExtent.width,
        surfaceCapabilities.maxImageExtent.height,
        surfaceCapabilities.currentExtent.width,
        surfaceCapabilities.currentExtent.height);

    ttlet currentExtentSet = (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) &&
        (surfaceCapabilities.currentExtent.height != std::numeric_limits<uint32_t>::max());

    if (!currentExtentSet) {
        // XXX On wayland, the window size is based on the size of the swapchain, so I need
        // to build a way of manual resizing the window outside of the operating system.
        LOG_FATAL("getSurfaceCapabilitiesKHR() does not supply currentExtent");
    }

    ttlet minImageCount = surfaceCapabilities.minImageCount;
    ttlet maxImageCount = surfaceCapabilities.maxImageCount ? surfaceCapabilities.maxImageCount : 10;
    ttlet imageCount = std::clamp(defaultNumberOfSwapchainImages, minImageCount, maxImageCount);
    LOG_INFO("minImageCount={}, maxImageCount={}, currentImageCount={}", minImageCount, maxImageCount, imageCount);
    return {imageCount, surfaceCapabilities.currentExtent};
}

bool gui_window_vulkan::readSurfaceExtent()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    try {
        std::tie(nrSwapchainImages, swapchainImageExtent) = getImageCountAndExtent();

    } catch (vk::SurfaceLostKHRError const &) {
        state = State::SurfaceLost;
        return false;
    }

    tt_axiom(widget);
    ttlet widget_size = widget->preferred_size();
    ttlet minimum_widget_size = widget_size.minimum();
    ttlet maximum_widget_size = widget_size.maximum();

    if (narrow_cast<int>(swapchainImageExtent.width) < minimum_widget_size.width() ||
        narrow_cast<int>(swapchainImageExtent.height) < minimum_widget_size.height()) {
        // Due to vulkan surface being extended across the window decoration;
        // On Windows 10 the swapchain-extent on a minimized window is no longer 0x0 instead
        // it is 160x28 pixels.

        // LOG_INFO("Window too small to draw current=({}, {}), minimum=({}, {})",
        //    swapchainImageExtent.width, swapchainImageExtent.height,
        //    minimumWindowExtent.width(), minimumWindowExtent.height()
        //);
        return false;
    }

    if (narrow_cast<int>(swapchainImageExtent.width) > maximum_widget_size.width() ||
        narrow_cast<int>(swapchainImageExtent.height) > maximum_widget_size.height()) {
        LOG_ERROR(
            "Window too large to draw current=({}, {}), maximum=({})",
            swapchainImageExtent.width,
            swapchainImageExtent.height,
            maximum_widget_size);
        return false;
    }

    return true;
}

bool gui_window_vulkan::checkSurfaceExtent()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    try {
        ttlet[nrImages, extent] = getImageCountAndExtent();
        return (nrImages == static_cast<uint32_t>(nrSwapchainImages)) && (extent == swapchainImageExtent);

    } catch (vk::SurfaceLostKHRError const &) {
        state = State::SurfaceLost;
        return false;
    }
}

void gui_window_vulkan::buildDevice()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
}

bool gui_window_vulkan::buildSurface()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    intrinsic = getSurface();
    return vulkan_device().score(intrinsic) > 0;
}

gui_window::State gui_window_vulkan::buildSwapchain()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    LOG_INFO("Building swap chain");

    ttlet sharingMode = vulkan_device().graphicsQueueFamilyIndex == vulkan_device().presentQueueFamilyIndex ?
        vk::SharingMode::eExclusive :
        vk::SharingMode::eConcurrent;

    std::array<uint32_t, 2> const sharingQueueFamilyAllIndices = {
        vulkan_device().graphicsQueueFamilyIndex, vulkan_device().presentQueueFamilyIndex};

    swapchainImageFormat = vulkan_device().bestSurfaceFormat;
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
        sharingMode == vk::SharingMode::eConcurrent ? narrow_cast<uint32_t>(sharingQueueFamilyAllIndices.size()) : 0,
        sharingMode == vk::SharingMode::eConcurrent ? sharingQueueFamilyAllIndices.data() : nullptr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vulkan_device().bestSurfacePresentMode,
        VK_TRUE, // clipped
        nullptr};

    vk::Result const result = vulkan_device().createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapchain);
    switch (result) {
    case vk::Result::eSuccess: break;

    case vk::Result::eErrorSurfaceLostKHR: return State::SurfaceLost;

    default: tt_error_info().set<vk_result_tag>(result); throw gui_error("Unknown result from createSwapchainKHR()");
    }

    LOG_INFO("Finished building swap chain");
    LOG_INFO(" - extent=({}, {})", swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height);
    LOG_INFO(
        " - colorSpace={}, format={}",
        vk::to_string(swapchainCreateInfo.imageColorSpace),
        vk::to_string(swapchainCreateInfo.imageFormat));
    LOG_INFO(
        " - presentMode={}, imageCount={}", vk::to_string(swapchainCreateInfo.presentMode), swapchainCreateInfo.minImageCount);

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
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vulkan_device().transientImageUsageFlags,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::ImageLayout::eUndefined};

    VmaAllocationCreateInfo depthAllocationCreateInfo = {};
    depthAllocationCreateInfo.usage = vulkan_device().lazyMemoryUsage;
    std::tie(depthImage, depthImageAllocation) = vulkan_device().createImage(depthImageCreateInfo, depthAllocationCreateInfo);

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
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment |
            vulkan_device().transientImageUsageFlags,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::ImageLayout::eUndefined};

    VmaAllocationCreateInfo colorAllocationCreateInfo = {};
    colorAllocationCreateInfo.usage = vulkan_device().lazyMemoryUsage;
    std::tie(colorImage, colorImageAllocation) = vulkan_device().createImage(colorImageCreateInfo, colorAllocationCreateInfo);

    return State::ReadyToRender;
}

void gui_window_vulkan::teardownSwapchain()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    vulkan_device().destroy(swapchain);
    vulkan_device().destroyImage(depthImage, depthImageAllocation);
    vulkan_device().destroyImage(colorImage, colorImageAllocation);
}

void gui_window_vulkan::buildFramebuffers()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    depthImageView = vulkan_device().createImageView(
        {vk::ImageViewCreateFlags(),
         depthImage,
         vk::ImageViewType::e2D,
         depthImageFormat,
         vk::ComponentMapping(),
         {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}});

    colorImageView = vulkan_device().createImageView(
        {vk::ImageViewCreateFlags(),
         colorImage,
         vk::ImageViewType::e2D,
         colorImageFormat,
         vk::ComponentMapping(),
         {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}});

    colorDescriptorImageInfo = {vk::Sampler(), colorImageView, vk::ImageLayout::eShaderReadOnlyOptimal};

    swapchainImages = vulkan_device().getSwapchainImagesKHR(swapchain);
    for (auto image : swapchainImages) {
        ttlet swapchainImageView = vulkan_device().createImageView(
            {vk::ImageViewCreateFlags(),
             image,
             vk::ImageViewType::e2D,
             swapchainImageFormat.format,
             vk::ComponentMapping(),
             {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}});

        swapchainImageViews.push_back(swapchainImageView);

        ttlet attachments = std::array{swapchainImageView, colorImageView, depthImageView};

        ttlet framebuffer = vulkan_device().createFramebuffer({
            vk::FramebufferCreateFlags(),
            renderPass,
            narrow_cast<uint32_t>(attachments.size()),
            attachments.data(),
            swapchainImageExtent.width,
            swapchainImageExtent.height,
            1 // layers
        });
        swapchainFramebuffers.push_back(framebuffer);

        swapchainRedrawRectangle.emplace_back();
    }

    tt_axiom(swapchainImageViews.size() == swapchainImages.size());
    tt_axiom(swapchainFramebuffers.size() == swapchainImages.size());
    tt_axiom(swapchainRedrawRectangle.size() == swapchainImages.size());
}

void gui_window_vulkan::teardownFramebuffers()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    swapchainRedrawRectangle.clear();

    for (auto frameBuffer : swapchainFramebuffers) {
        vulkan_device().destroy(frameBuffer);
    }
    swapchainFramebuffers.clear();

    for (auto imageView : swapchainImageViews) {
        vulkan_device().destroy(imageView);
    }
    swapchainImageViews.clear();

    vulkan_device().destroy(depthImageView);
    vulkan_device().destroy(colorImageView);
}

void gui_window_vulkan::buildRenderPasses()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    ttlet attachmentDescriptions = std::array{
        vk::AttachmentDescription{
            // Swapchain attachment.
            vk::AttachmentDescriptionFlags(),
            swapchainImageFormat.format,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
            vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
            vk::ImageLayout::ePresentSrcKHR, // initialLayout
            vk::ImageLayout::ePresentSrcKHR // finalLayout

        },
        vk::AttachmentDescription{
            // Color attachment
            vk::AttachmentDescriptionFlags(),
            colorImageFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
            vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
            vk::ImageLayout::eUndefined, // initialLayout
            vk::ImageLayout::eColorAttachmentOptimal // finalLayout

        },
        vk::AttachmentDescription{
            // Depth attachment
            vk::AttachmentDescriptionFlags(),
            depthImageFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
            vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
            vk::ImageLayout::eUndefined, // initialLayout
            vk::ImageLayout::eDepthStencilAttachmentOptimal // finalLayout
        }};

    ttlet colorAttachmentReferences = std::array{vk::AttachmentReference{1, vk::ImageLayout::eColorAttachmentOptimal}};

    ttlet colorInputAttachmentReferences = std::array{vk::AttachmentReference{1, vk::ImageLayout::eShaderReadOnlyOptimal}};

    ttlet swapchainAttachmentReferences = std::array{vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}};

    ttlet depthAttachmentReference = vk::AttachmentReference{2, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    ttlet subpassDescriptions = std::array{
        vk::SubpassDescription{// Subpass 0
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               0, // inputAttchmentReferencesCount
                               nullptr, // inputAttachmentReferences
                               narrow_cast<uint32_t>(colorAttachmentReferences.size()),
                               colorAttachmentReferences.data(),
                               nullptr, // resolveAttachments
                               &depthAttachmentReference

        },
        vk::SubpassDescription{// Subpass 1
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               0, // inputAttchmentReferencesCount
                               nullptr, // inputAttachmentReferences
                               narrow_cast<uint32_t>(colorAttachmentReferences.size()),
                               colorAttachmentReferences.data(),
                               nullptr, // resolveAttachments
                               &depthAttachmentReference

        },
        vk::SubpassDescription{// Subpass 2
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               0, // inputAttchmentReferencesCount
                               nullptr, // inputAttachmentReferences
                               narrow_cast<uint32_t>(colorAttachmentReferences.size()),
                               colorAttachmentReferences.data(),
                               nullptr, // resolveAttachments
                               &depthAttachmentReference

        },
        vk::SubpassDescription{// Subpass 3
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               narrow_cast<uint32_t>(colorInputAttachmentReferences.size()),
                               colorInputAttachmentReferences.data(),
                               narrow_cast<uint32_t>(colorAttachmentReferences.size()),
                               colorAttachmentReferences.data(),
                               nullptr, // resolveAttachments
                               &depthAttachmentReference

        },
        vk::SubpassDescription{// Subpass 4 tone-mapper
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               narrow_cast<uint32_t>(colorInputAttachmentReferences.size()),
                               colorInputAttachmentReferences.data(),
                               narrow_cast<uint32_t>(swapchainAttachmentReferences.size()),
                               swapchainAttachmentReferences.data(),
                               nullptr,
                               nullptr}};

    ttlet subpassDependency = std::array{
        vk::SubpassDependency{
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eMemoryRead,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 0: Render single color polygons to color+depth attachment.
        vk::SubpassDependency{
            0,
            1,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentRead,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 1: Render shaded polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            1,
            2,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentRead,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 2: Render texture mapped polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            2,
            3,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eShaderRead,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 3: Render SDF-texture mapped polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            3,
            4,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eShaderRead,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 4: Tone mapping color to swapchain.
        vk::SubpassDependency{
            4,
            VK_SUBPASS_EXTERNAL,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eMemoryRead,
            vk::DependencyFlagBits::eByRegion}};

    vk::RenderPassCreateInfo const renderPassCreateInfo = {
        vk::RenderPassCreateFlags(),
        narrow_cast<uint32_t>(attachmentDescriptions.size()), // attachmentCount
        attachmentDescriptions.data(), // attachments
        narrow_cast<uint32_t>(subpassDescriptions.size()), // subpassCount
        subpassDescriptions.data(), // subpasses
        narrow_cast<uint32_t>(subpassDependency.size()), // dependencyCount
        subpassDependency.data() // dependencies
    };

    renderPass = vulkan_device().createRenderPass(renderPassCreateInfo);
}

void gui_window_vulkan::teardownRenderPasses()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    vulkan_device().destroy(renderPass);
}

void gui_window_vulkan::buildSemaphores()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    imageAvailableSemaphore = vulkan_device().createSemaphore({});
    renderFinishedSemaphore = vulkan_device().createSemaphore({});

    // This fence is used to wait for the Window and its Pipelines to be idle.
    // It should therefor be signed at the start so that when no rendering has been
    // done it is still idle.
    renderFinishedFence = vulkan_device().createFence({vk::FenceCreateFlagBits::eSignaled});
}

void gui_window_vulkan::teardownSemaphores()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    vulkan_device().destroy(renderFinishedSemaphore);
    vulkan_device().destroy(imageAvailableSemaphore);
    vulkan_device().destroy(renderFinishedFence);
}

void gui_window_vulkan::buildCommandBuffers()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    ttlet commandBuffers =
        vulkan_device().allocateCommandBuffers({vulkan_device().graphicsCommandPool, vk::CommandBufferLevel::ePrimary, 1});

    commandBuffer = commandBuffers.at(0);
}

void gui_window_vulkan::teardownCommandBuffers()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    ttlet commandBuffers = std::vector<vk::CommandBuffer>{commandBuffer};

    vulkan_device().freeCommandBuffers(vulkan_device().graphicsCommandPool, commandBuffers);
}

void gui_window_vulkan::teardownSurface()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    narrow_cast<gui_system_vulkan &>(system).destroySurfaceKHR(intrinsic);
}

void gui_window_vulkan::teardownDevice()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    _device = nullptr;
}

} // namespace tt
