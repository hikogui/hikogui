// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_surface_vulkan.hpp"
#include "gfx_system_vulkan.hpp"
#include "gfx_device_vulkan.hpp"
#include "pipeline_flat.hpp"
#include "pipeline_box.hpp"
#include "pipeline_image.hpp"
#include "pipeline_SDF.hpp"
#include "pipeline_tone_mapper.hpp"
#include "draw_context.hpp"
#include "../widgets/window_widget.hpp"
#include "../trace.hpp"
#include "../application.hpp"
#include "../cast.hpp"
#include <vector>

namespace tt {

using namespace std;

gfx_surface_vulkan::gfx_surface_vulkan(gfx_system &system, vk::SurfaceKHR surface) :
    gfx_surface(system), intrinsic(surface)
{
}

gfx_surface_vulkan::~gfx_surface_vulkan() {
    if (state != gfx_surface_state::no_window) {
        tt_log_fatal("The window attached to the gfx_surface still exists during destruction.");
    }
}

gfx_device_vulkan &gfx_surface_vulkan::vulkan_device() const noexcept
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());
    tt_axiom(_device != nullptr);
    return narrow_cast<gfx_device_vulkan &>(*_device);
}

void gfx_surface_vulkan::init()
{
    ttlet lock = std::scoped_lock(gfx_system_mutex);

    gfx_surface::init();
    flatPipeline = std::make_unique<pipeline_flat::pipeline_flat>(*this);
    boxPipeline = std::make_unique<pipeline_box::pipeline_box>(*this);
    imagePipeline = std::make_unique<pipeline_image::pipeline_image>(*this);
    SDFPipeline = std::make_unique<pipeline_SDF::pipeline_SDF>(*this);
    toneMapperPipeline = std::make_unique<pipeline_tone_mapper::pipeline_tone_mapper>(*this);
}

void gfx_surface_vulkan::waitIdle()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    tt_assert(_device);
    if (renderFinishedFence) {
        vulkan_device().waitForFences({renderFinishedFence}, VK_TRUE, std::numeric_limits<uint64_t>::max());
    }
    vulkan_device().waitIdle();
    tt_log_info("/waitIdle");
}

std::optional<uint32_t> gfx_surface_vulkan::acquireNextImageFromSwapchain()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    // swap chain, fence & imageAvailableSemaphore must be externally synchronized.
    uint32_t frameBufferIndex = 0;
    // tt_log_debug("acquireNextImage '{}'", title);

    ttlet result = vulkan_device().acquireNextImageKHR(swapchain, 0, imageAvailableSemaphore, vk::Fence(), &frameBufferIndex);
    // tt_log_debug("acquireNextImage {}", frameBufferIndex);

    switch (result) {
    case vk::Result::eSuccess: return {frameBufferIndex};

    case vk::Result::eSuboptimalKHR:
        tt_log_info("acquireNextImageKHR() eSuboptimalKHR");
        state = gfx_surface_state::swapchain_lost;
        return {};

    case vk::Result::eErrorOutOfDateKHR:
        tt_log_info("acquireNextImageKHR() eErrorOutOfDateKHR");
        state = gfx_surface_state::swapchain_lost;
        return {};

    case vk::Result::eErrorSurfaceLostKHR:
        tt_log_info("acquireNextImageKHR() eErrorSurfaceLostKHR");
        state = gfx_surface_state::surface_lost;
        return {};

    case vk::Result::eTimeout:
        // Don't render, we didn't receive an image.
        tt_log_info("acquireNextImageKHR() eTimeout");
        return {};

    default: throw gui_error("Unknown result from acquireNextImageKHR(). '{}'", to_string(result));
    }
}

void gfx_surface_vulkan::presentImageToQueue(uint32_t frameBufferIndex, vk::Semaphore semaphore)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    tt_axiom(_device);

    std::array<vk::Semaphore, 1> const renderFinishedSemaphores = {semaphore};
    std::array<vk::SwapchainKHR, 1> const presentSwapchains = {swapchain};
    std::array<uint32_t, 1> const presentImageIndices = {frameBufferIndex};
    tt_axiom(presentSwapchains.size() == presentImageIndices.size());

    try {
        // tt_log_debug("presentQueue {}", presentImageIndices.at(0));
        ttlet result = vulkan_device().presentQueue.presentKHR(
            {narrow_cast<uint32_t>(renderFinishedSemaphores.size()),
             renderFinishedSemaphores.data(),
             narrow_cast<uint32_t>(presentSwapchains.size()),
             presentSwapchains.data(),
             presentImageIndices.data()});

        switch (result) {
        case vk::Result::eSuccess: return;

        case vk::Result::eSuboptimalKHR:
            tt_log_info("presentKHR() eSuboptimalKHR");
            state = gfx_surface_state::swapchain_lost;
            return;

        default: throw gui_error("Unknown result from presentKHR(). '{}'", to_string(result));
        }

    } catch (vk::OutOfDateKHRError const &) {
        tt_log_info("presentKHR() eErrorOutOfDateKHR");
        state = gfx_surface_state::swapchain_lost;
        return;

    } catch (vk::SurfaceLostKHRError const &) {
        tt_log_info("presentKHR() eErrorSurfaceLostKHR");
        state = gfx_surface_state::surface_lost;
        return;
    }
}

void gfx_surface_vulkan::build(extent2 minimum_size, extent2 maximum_size)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    if (state == gfx_surface_state::no_device) {
        if (_device) {
            flatPipeline->buildForNewDevice();
            boxPipeline->buildForNewDevice();
            imagePipeline->buildForNewDevice();
            SDFPipeline->buildForNewDevice();
            toneMapperPipeline->buildForNewDevice();
            state = gfx_surface_state::no_surface;
        }
    }

    if (state == gfx_surface_state::no_surface) {
        if (!buildSurface()) {
            state = gfx_surface_state::device_lost;
            return;
        }
        flatPipeline->buildForNewSurface();
        boxPipeline->buildForNewSurface();
        imagePipeline->buildForNewSurface();
        SDFPipeline->buildForNewSurface();
        toneMapperPipeline->buildForNewSurface();
        state = gfx_surface_state::no_swapchain;
    }

    if (state == gfx_surface_state::no_swapchain) {
        if (!readSurfaceExtent(minimum_size, maximum_size)) {
            // Minimized window, can not build a new swap chain.
            state = gfx_surface_state::no_swapchain;
            return;
        }

        ttlet s = buildSwapchain();
        if (s != gfx_surface_state::ready_to_render) {
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
        tt_axiom(flatPipeline);
        tt_axiom(boxPipeline);
        tt_axiom(imagePipeline);
        tt_axiom(SDFPipeline);
        tt_axiom(toneMapperPipeline);
        flatPipeline->buildForNewSwapchain(renderPass, 0, swapchainImageExtent);
        boxPipeline->buildForNewSwapchain(renderPass, 1, swapchainImageExtent);
        imagePipeline->buildForNewSwapchain(renderPass, 2, swapchainImageExtent);
        SDFPipeline->buildForNewSwapchain(renderPass, 3, swapchainImageExtent);
        toneMapperPipeline->buildForNewSwapchain(renderPass, 4, swapchainImageExtent);

        size = {narrow_cast<float>(swapchainImageExtent.width), narrow_cast<float>(swapchainImageExtent.height)};
        state = gfx_surface_state::ready_to_render;
    }
}

void gfx_surface_vulkan::teardown()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    auto nextState = state;

    if (state >= gfx_surface_state::swapchain_lost) {
        tt_log_info("Tearing down because the window lost the swapchain.");
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
        nextState = gfx_surface_state::no_swapchain;

        if (state >= gfx_surface_state::surface_lost) {
            tt_log_info("Tearing down because the window lost the drawable surface.");
            toneMapperPipeline->teardownForSurfaceLost();
            SDFPipeline->teardownForSurfaceLost();
            imagePipeline->teardownForSurfaceLost();
            boxPipeline->teardownForSurfaceLost();
            flatPipeline->teardownForSurfaceLost();
            teardownSurface();
            nextState = gfx_surface_state::no_surface;

            if (state >= gfx_surface_state::device_lost) {
                tt_log_info("Tearing down because the window lost the vulkan device.");

                toneMapperPipeline->teardownForDeviceLost();
                SDFPipeline->teardownForDeviceLost();
                imagePipeline->teardownForDeviceLost();
                boxPipeline->teardownForDeviceLost();
                flatPipeline->teardownForDeviceLost();
                teardownDevice();
                nextState = gfx_surface_state::no_device;

                if (state >= gfx_surface_state::window_lost) {
                    tt_log_info("Tearing down because the window doesn't exist anymore.");

                    toneMapperPipeline->teardownForWindowLost();
                    SDFPipeline->teardownForWindowLost();
                    imagePipeline->teardownForWindowLost();
                    boxPipeline->teardownForWindowLost();
                    flatPipeline->teardownForWindowLost();
                    nextState = gfx_surface_state::no_window;
                }
            }
        }
    }
    state = nextState;
}

[[nodiscard]] extent2 gfx_surface_vulkan::update(extent2 minimum_size, extent2 maximum_size) noexcept
{
    // Tear down then buildup from the Vulkan objects that where invalid.
    teardown();
    build(minimum_size, maximum_size);
    return size;
}

std::optional<draw_context> gfx_surface_vulkan::render_start(aarectangle redraw_rectangle)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    // Bail out when the window is not yet ready to be rendered, or if there is nothing to render.
    if (state != gfx_surface_state::ready_to_render || !redraw_rectangle) {
        return {};
    }

    ttlet optional_frame_buffer_index = acquireNextImageFromSwapchain();
    if (!optional_frame_buffer_index) {
        // No image is ready to be rendered, yet, possibly because our vertical sync function
        // is not working correctly.
        return {};
    }

    ttlet frame_buffer_index = *optional_frame_buffer_index;
    auto &current_image = swapchain_image_infos.at(frame_buffer_index);

    // Record which part of the image will be redrawn on the current swapchain image.
    current_image.redraw_rectangle = redraw_rectangle;

    // Calculate the scissor rectangle, from the combined redraws of the complete swapchain.
    // We need to do this so that old redraws are also executed in the current swapchain image.
    ttlet scissor_rectangle = ceil(
        std::accumulate(swapchain_image_infos.cbegin(), swapchain_image_infos.cend(), aarectangle{}, [](ttlet &sum, ttlet &item) {
            return sum | item.redraw_rectangle;
        }));

    // Wait until previous rendering has finished, before the next rendering.
    vulkan_device().waitForFences({renderFinishedFence}, VK_TRUE, std::numeric_limits<uint64_t>::max());

    // Unsignal the fence so we will not modify/destroy the command buffers during rendering.
    vulkan_device().resetFences({renderFinishedFence});

    // Update the widgets before the pipelines need their vertices.
    // We unset modified before, so that modification requests are captured.
    return draw_context{
        *narrow_cast<gfx_device_vulkan *>(_device),
        narrow_cast<size_t>(frame_buffer_index),
        size,
        scissor_rectangle,
        flatPipeline->vertexBufferData,
        boxPipeline->vertexBufferData,
        imagePipeline->vertexBufferData,
        SDFPipeline->vertexBufferData};
}

void gfx_surface_vulkan::render_finish(draw_context const &context, color background_color)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    auto &current_image = swapchain_image_infos.at(context.frame_buffer_index());

    fill_command_buffer(current_image, context.scissor_rectangle(), background_color);
    submitCommandBuffer();

    // Signal the fence when all rendering has finished on the graphics queue.
    // When the fence is signaled we can modify/destroy the command buffers.
    [[maybe_unused]] ttlet submit_result = vulkan_device().graphicsQueue.submit(0, nullptr, renderFinishedFence);

    presentImageToQueue(narrow_cast<uint32_t>(context.frame_buffer_index()), renderFinishedSemaphore);

    // Do an early tear down of invalid vulkan objects.
    teardown();
}

void gfx_surface_vulkan::fill_command_buffer(swapchain_image_info &current_image, aarectangle scissor_rectangle, color background_color)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    auto t = trace<"fill_command_buffer">{};

    commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    ttlet background_color_f32x4 = static_cast<f32x4>(background_color);
    ttlet background_color_array = static_cast<std::array<float, 4>>(background_color_f32x4);

    ttlet colorClearValue = vk::ClearColorValue{background_color_array};
    ttlet sdfClearValue = vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}};
    ttlet depthClearValue = vk::ClearDepthStencilValue{0.0, 0};
    ttlet clearValues = std::array{
        vk::ClearValue{depthClearValue},
        vk::ClearValue{colorClearValue},
        vk::ClearValue{sdfClearValue},
        vk::ClearValue{colorClearValue}};

    // Clamp the scissor rectangle to the size of the window.
    scissor_rectangle = intersect(
        scissor_rectangle,
        aarectangle{0.0f, 0.0f, narrow_cast<float>(swapchainImageExtent.width), narrow_cast<float>(swapchainImageExtent.height)});
    scissor_rectangle = ceil(scissor_rectangle);

    ttlet scissors = std::array{vk::Rect2D{
        vk::Offset2D(
            narrow_cast<uint32_t>(scissor_rectangle.left()),
            narrow_cast<uint32_t>(swapchainImageExtent.height - scissor_rectangle.bottom() - scissor_rectangle.height())),
        vk::Extent2D(narrow_cast<uint32_t>(scissor_rectangle.width()), narrow_cast<uint32_t>(scissor_rectangle.height()))}};

    // The scissor and render area makes sure that the frame buffer is not modified where we are not drawing the widgets.
    commandBuffer.setScissor(0, scissors);

    ttlet renderArea = scissors.at(0);

    // Because we use a scissor the image from the swapchain around the scissor-area is reused.
    // Because of reuse the swapchain image must already be in the "ePresentSrcKHR" layout.
    // The swapchain creates images in undefined layout, so we need to change the layout once.
    if (not current_image.layout_is_present) {
        gfx_device_vulkan::transition_layout(commandBuffer,
            current_image.image, swapchainImageFormat.format, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

        current_image.layout_is_present = true;
    }

    commandBuffer.beginRenderPass(
        {renderPass, current_image.frame_buffer, renderArea, narrow_cast<uint32_t>(clearValues.size()), clearValues.data()},
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

void gfx_surface_vulkan::submitCommandBuffer()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

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

std::tuple<uint32_t, vk::Extent2D> gfx_surface_vulkan::getImageCountAndExtent()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    surfaceCapabilities = vulkan_device().getSurfaceCapabilitiesKHR(intrinsic);

    tt_log_info(
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
        tt_log_fatal("getSurfaceCapabilitiesKHR() does not supply currentExtent");
    }

    ttlet minImageCount = surfaceCapabilities.minImageCount;
    ttlet maxImageCount = surfaceCapabilities.maxImageCount ? surfaceCapabilities.maxImageCount : 10;
    ttlet imageCount = std::clamp(defaultNumberOfSwapchainImages, minImageCount, maxImageCount);
    tt_log_info("minImageCount={}, maxImageCount={}, currentImageCount={}", minImageCount, maxImageCount, imageCount);
    return {imageCount, surfaceCapabilities.currentExtent};
}

bool gfx_surface_vulkan::readSurfaceExtent(extent2 minimum_size, extent2 maximum_size)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    try {
        std::tie(nrSwapchainImages, swapchainImageExtent) = getImageCountAndExtent();

    } catch (vk::SurfaceLostKHRError const &) {
        state = gfx_surface_state::surface_lost;
        return false;
    }

    if (narrow_cast<float>(swapchainImageExtent.width) < minimum_size.width() ||
        narrow_cast<float>(swapchainImageExtent.height) < minimum_size.height()) {
        // Due to vulkan surface being extended across the window decoration;
        // On Windows 10 the swapchain-extent on a minimized window is no longer 0x0 instead
        // it is 160x28 pixels.

        tt_log_info("Window too small ({}, {}) to draw widgets requiring a window size between {} and {}.",
            swapchainImageExtent.width, swapchainImageExtent.height,
            minimum_size,
            maximum_size
        );
        return false;
    }

    if (narrow_cast<int>(swapchainImageExtent.width) > maximum_size.width() ||
        narrow_cast<int>(swapchainImageExtent.height) > maximum_size.height()) {
        tt_log_error(
            "Window too large ({}, {}) to draw widgets requiring a window size between {} and {}",
            swapchainImageExtent.width,
            swapchainImageExtent.height,
            minimum_size,
            maximum_size);
        return false;
    }

    return true;
}

bool gfx_surface_vulkan::checkSurfaceExtent()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    try {
        ttlet[nrImages, extent_] = getImageCountAndExtent();
        return (nrImages == static_cast<uint32_t>(nrSwapchainImages)) && (extent_ == swapchainImageExtent);

    } catch (vk::SurfaceLostKHRError const &) {
        state = gfx_surface_state::surface_lost;
        return false;
    }
}

void gfx_surface_vulkan::buildDevice()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());
}

bool gfx_surface_vulkan::buildSurface()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    return vulkan_device().score(intrinsic) > 0;
}

gfx_surface_state gfx_surface_vulkan::buildSwapchain()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    tt_log_info("Building swap chain");

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

    case vk::Result::eErrorSurfaceLostKHR: return gfx_surface_state::surface_lost;

    default: throw gui_error("Unknown result from createSwapchainKHR(). '{}'", to_string(result));
    }

    tt_log_info("Finished building swap chain");
    tt_log_info(" - extent=({}, {})", swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height);
    tt_log_info(
        " - colorSpace={}, format={}",
        vk::to_string(swapchainCreateInfo.imageColorSpace),
        vk::to_string(swapchainCreateInfo.imageFormat));
    tt_log_info(
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
    std::tie(colorImages[0], colorImageAllocations[0]) =
        vulkan_device().createImage(colorImageCreateInfo, colorAllocationCreateInfo);
    std::tie(colorImages[1], colorImageAllocations[1]) =
        vulkan_device().createImage(colorImageCreateInfo, colorAllocationCreateInfo);

    return gfx_surface_state::ready_to_render;
}

void gfx_surface_vulkan::teardownSwapchain()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    vulkan_device().destroy(swapchain);
    vulkan_device().destroyImage(depthImage, depthImageAllocation);

    for (size_t i = 0; i != std::size(colorImages); ++i) {
        vulkan_device().destroyImage(colorImages[i], colorImageAllocations[i]);
    }
}

void gfx_surface_vulkan::buildFramebuffers()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    depthImageView = vulkan_device().createImageView(
        {vk::ImageViewCreateFlags(),
         depthImage,
         vk::ImageViewType::e2D,
         depthImageFormat,
         vk::ComponentMapping(),
         {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}});

    for (size_t i = 0; i != std::size(colorImageViews); ++i) {
        colorImageViews[i] = vulkan_device().createImageView(
            {vk::ImageViewCreateFlags(),
             colorImages[i],
             vk::ImageViewType::e2D,
             colorImageFormat,
             vk::ComponentMapping(),
             {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}});

        colorDescriptorImageInfos[i] = {vk::Sampler(), colorImageViews[i], vk::ImageLayout::eShaderReadOnlyOptimal};
    }

    auto swapchain_images = vulkan_device().getSwapchainImagesKHR(swapchain);
    for (auto image : swapchain_images) {
        auto image_view = vulkan_device().createImageView(
            {vk::ImageViewCreateFlags(),
             image,
             vk::ImageViewType::e2D,
             swapchainImageFormat.format,
             vk::ComponentMapping(),
             {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}});

        ttlet attachments = std::array{depthImageView, colorImageViews[0], colorImageViews[1], image_view};

        ttlet frame_buffer = vulkan_device().createFramebuffer({
            vk::FramebufferCreateFlags(),
            renderPass,
            narrow_cast<uint32_t>(attachments.size()),
            attachments.data(),
            swapchainImageExtent.width,
            swapchainImageExtent.height,
            1 // layers
        });

        swapchain_image_infos.emplace_back(
            std::move(image), std::move(image_view), std::move(frame_buffer), aarectangle{}, false);
    }

    tt_axiom(swapchain_image_infos.size() == swapchain_images.size());
}

void gfx_surface_vulkan::teardownFramebuffers()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    for (auto &info : swapchain_image_infos) {
        vulkan_device().destroy(info.frame_buffer);
        vulkan_device().destroy(info.image_view);
    }
    swapchain_image_infos.clear();

    vulkan_device().destroy(depthImageView);
    for (size_t i = 0; i != std::size(colorImageViews); ++i) {
        vulkan_device().destroy(colorImageViews[i]);
    }
}

void gfx_surface_vulkan::buildRenderPasses()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    ttlet attachmentDescriptions = std::array{
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
        },
        vk::AttachmentDescription{
            // Color1 attachment
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
            // Color2 attachment
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
        }};

    ttlet depthAttachmentReference = vk::AttachmentReference{0, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    ttlet color1AttachmentReferences = std::array{vk::AttachmentReference{1, vk::ImageLayout::eColorAttachmentOptimal}};
    ttlet color2AttachmentReferences = std::array{vk::AttachmentReference{2, vk::ImageLayout::eColorAttachmentOptimal}};

    ttlet color1InputAttachmentReferences = std::array{vk::AttachmentReference{1, vk::ImageLayout::eShaderReadOnlyOptimal}};
    ttlet color2InputAttachmentReferences = std::array{vk::AttachmentReference{2, vk::ImageLayout::eShaderReadOnlyOptimal}};
    ttlet color12InputAttachmentReferences = std::array{
        vk::AttachmentReference{1, vk::ImageLayout::eShaderReadOnlyOptimal},
        vk::AttachmentReference{2, vk::ImageLayout::eShaderReadOnlyOptimal}};

    ttlet swapchainAttachmentReferences = std::array{vk::AttachmentReference{3, vk::ImageLayout::eColorAttachmentOptimal}};

    ttlet subpassDescriptions = std::array{
        vk::SubpassDescription{// Subpass 0
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               0, // inputAttchmentReferencesCount
                               nullptr, // inputAttachmentReferences
                               narrow_cast<uint32_t>(color1AttachmentReferences.size()),
                               color1AttachmentReferences.data(),
                               nullptr, // resolveAttachments
                               &depthAttachmentReference

        },
        vk::SubpassDescription{// Subpass 1
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               0, // inputAttchmentReferencesCount
                               nullptr, // inputAttachmentReferences
                               narrow_cast<uint32_t>(color1AttachmentReferences.size()),
                               color1AttachmentReferences.data(),
                               nullptr, // resolveAttachments
                               &depthAttachmentReference

        },
        vk::SubpassDescription{// Subpass 2
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               0, // inputAttchmentReferencesCount
                               nullptr, // inputAttachmentReferences
                               narrow_cast<uint32_t>(color1AttachmentReferences.size()),
                               color1AttachmentReferences.data(),
                               nullptr, // resolveAttachments
                               &depthAttachmentReference

        },
        vk::SubpassDescription{// Subpass 3
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               narrow_cast<uint32_t>(color1InputAttachmentReferences.size()),
                               color1InputAttachmentReferences.data(),
                               narrow_cast<uint32_t>(color2AttachmentReferences.size()),
                               color2AttachmentReferences.data(),
                               nullptr, // resolveAttachments
                               &depthAttachmentReference

        },
        vk::SubpassDescription{// Subpass 4 tone-mapper
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               narrow_cast<uint32_t>(color12InputAttachmentReferences.size()),
                               color12InputAttachmentReferences.data(),
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
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentRead,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 1: Render shaded polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            1,
            2,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
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
            vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 3: Render SDF-texture mapped polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            3,
            4,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 4: Tone mapping color to swapchain.
        vk::SubpassDependency{
            4,
            VK_SUBPASS_EXTERNAL,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
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

void gfx_surface_vulkan::teardownRenderPasses()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    vulkan_device().destroy(renderPass);
}

void gfx_surface_vulkan::buildSemaphores()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    imageAvailableSemaphore = vulkan_device().createSemaphore({});
    renderFinishedSemaphore = vulkan_device().createSemaphore({});

    // This fence is used to wait for the Window and its Pipelines to be idle.
    // It should therefor be signed at the start so that when no rendering has been
    // done it is still idle.
    renderFinishedFence = vulkan_device().createFence({vk::FenceCreateFlagBits::eSignaled});
}

void gfx_surface_vulkan::teardownSemaphores()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    vulkan_device().destroy(renderFinishedSemaphore);
    vulkan_device().destroy(imageAvailableSemaphore);
    vulkan_device().destroy(renderFinishedFence);
}

void gfx_surface_vulkan::buildCommandBuffers()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    ttlet commandBuffers =
        vulkan_device().allocateCommandBuffers({vulkan_device().graphicsCommandPool, vk::CommandBufferLevel::ePrimary, 1});

    commandBuffer = commandBuffers.at(0);
}

void gfx_surface_vulkan::teardownCommandBuffers()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());
    ttlet commandBuffers = std::vector<vk::CommandBuffer>{commandBuffer};

    vulkan_device().freeCommandBuffers(vulkan_device().graphicsCommandPool, commandBuffers);
}

void gfx_surface_vulkan::teardownSurface()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    narrow_cast<gfx_system_vulkan &>(system).destroySurfaceKHR(intrinsic);
}

void gfx_surface_vulkan::teardownDevice()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    _device = nullptr;
}

} // namespace tt
