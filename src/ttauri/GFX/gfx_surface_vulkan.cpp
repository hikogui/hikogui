// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_surface_vulkan.hpp"
#include "gfx_system_vulkan.hpp"
#include "gfx_device_vulkan.hpp"
#include "pipeline_box.hpp"
#include "pipeline_image.hpp"
#include "pipeline_SDF.hpp"
#include "pipeline_tone_mapper.hpp"
#include "../widgets/window_widget.hpp"
#include "../trace.hpp"
#include "../cast.hpp"
#include <vector>

namespace tt::inline v1 {

gfx_surface_vulkan::gfx_surface_vulkan(gfx_system &system, vk::SurfaceKHR surface) : gfx_surface(system), intrinsic(surface) {}

gfx_surface_vulkan::~gfx_surface_vulkan()
{
    if (state != gfx_surface_state::no_window) {
        tt_log_fatal("The window attached to the gfx_surface still exists during destruction.");
    }
}

void gfx_surface_vulkan::set_device(gfx_device *device) noexcept
{
    tt_axiom(device);

    ttlet lock = std::scoped_lock(gfx_system_mutex);
    super::set_device(device);

    auto device_ = down_cast<gfx_device_vulkan *>(device);
    _present_queue = &device_->get_present_queue(*this);
    _graphics_queue = &device_->get_graphics_queue(*this);
}

gfx_device_vulkan &gfx_surface_vulkan::vulkan_device() const noexcept
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());
    tt_axiom(_device != nullptr);
    return down_cast<gfx_device_vulkan &>(*_device);
}

void gfx_surface_vulkan::init()
{
    ttlet lock = std::scoped_lock(gfx_system_mutex);

    gfx_surface::init();
    boxPipeline = std::make_unique<pipeline_box::pipeline_box>(*this);
    imagePipeline = std::make_unique<pipeline_image::pipeline_image>(*this);
    SDFPipeline = std::make_unique<pipeline_SDF::pipeline_SDF>(*this);
    toneMapperPipeline = std::make_unique<pipeline_tone_mapper::pipeline_tone_mapper>(*this);
}

[[nodiscard]] extent2 gfx_surface_vulkan::size() const noexcept
{
    return extent2{narrow_cast<float>(swapchainImageExtent.width), narrow_cast<float>(swapchainImageExtent.height)};
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
        ttlet result = _present_queue->queue.presentKHR(
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

void gfx_surface_vulkan::build(extent2 new_size)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    if (state == gfx_surface_state::no_device) {
        if (_device) {
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
        boxPipeline->buildForNewSurface();
        imagePipeline->buildForNewSurface();
        SDFPipeline->buildForNewSurface();
        toneMapperPipeline->buildForNewSurface();
        state = gfx_surface_state::no_swapchain;
    }

    if (state == gfx_surface_state::no_swapchain) {
        try {
            ttlet[clamped_count, clamped_size] = get_image_count_and_size(defaultNumberOfSwapchainImages, new_size);
            if (not new_size) {
                // Minimized window, can not build a new swap chain.
                state = gfx_surface_state::no_swapchain;
                return;
            }

            ttlet s = buildSwapchain(clamped_count, clamped_size);
            if (s != gfx_surface_state::ready_to_render) {
                state = s;
                return;
            }

            ttlet[clamped_count_check, clamped_size_check] = get_image_count_and_size(clamped_count, clamped_size);
            if (clamped_count_check != clamped_count or clamped_size_check != clamped_size) {
                // Window has changed during swap chain creation, it is in a inconsistent bad state.
                // This is a bug in the Vulkan specification.
                teardownSwapchain();
                state = gfx_surface_state::no_swapchain;
                return;
            }

            buildRenderPasses(); // Render-pass requires the swapchain/color/depth image-format.
            buildFramebuffers(); // Framebuffer required render passes.
            buildCommandBuffers();
            buildSemaphores();
            tt_axiom(boxPipeline);
            tt_axiom(imagePipeline);
            tt_axiom(SDFPipeline);
            tt_axiom(toneMapperPipeline);
            boxPipeline->buildForNewSwapchain(renderPass, 0, swapchainImageExtent);
            imagePipeline->buildForNewSwapchain(renderPass, 1, swapchainImageExtent);
            SDFPipeline->buildForNewSwapchain(renderPass, 2, swapchainImageExtent);
            toneMapperPipeline->buildForNewSwapchain(renderPass, 3, swapchainImageExtent);

            state = gfx_surface_state::ready_to_render;

        } catch (vk::SurfaceLostKHRError const &) {
            // During swapchain build we lost the surface.
            // This state will cause the swapchain to be teardown.
            state = gfx_surface_state::surface_lost;
            return;
        }
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
            teardownSurface();
            nextState = gfx_surface_state::no_surface;

            if (state >= gfx_surface_state::device_lost) {
                tt_log_info("Tearing down because the window lost the vulkan device.");

                toneMapperPipeline->teardownForDeviceLost();
                SDFPipeline->teardownForDeviceLost();
                imagePipeline->teardownForDeviceLost();
                boxPipeline->teardownForDeviceLost();
                teardownDevice();
                nextState = gfx_surface_state::no_device;

                if (state >= gfx_surface_state::window_lost) {
                    tt_log_info("Tearing down because the window doesn't exist anymore.");

                    toneMapperPipeline->teardownForWindowLost();
                    SDFPipeline->teardownForWindowLost();
                    imagePipeline->teardownForWindowLost();
                    boxPipeline->teardownForWindowLost();
                    nextState = gfx_surface_state::no_window;
                }
            }
        }
    }
    state = nextState;
}

void gfx_surface_vulkan::update(extent2 new_size) noexcept
{
    ttlet lock = std::scoped_lock(gfx_system_mutex);

    if (size() != new_size and state == gfx_surface_state::ready_to_render) {
        // On resize lose the swapchain, which will be cleaned up at teardown().
        state = gfx_surface_state::swapchain_lost;
    }

    // Tear down then buildup from the Vulkan objects that where invalid.
    teardown();
    build(new_size);
}

draw_context gfx_surface_vulkan::render_start(aarectangle redraw_rectangle)
{
    // Extent the redraw_rectangle to the render-area-granularity to improve performance on tile based GPUs.
    redraw_rectangle = ceil(redraw_rectangle, _render_area_granularity);

    ttlet lock = std::scoped_lock(gfx_system_mutex);

    auto r = draw_context{
        *down_cast<gfx_device_vulkan *>(_device),
        boxPipeline->vertexBufferData,
        imagePipeline->vertexBufferData,
        SDFPipeline->vertexBufferData};

    // Bail out when the window is not yet ready to be rendered, or if there is nothing to render.
    if (state != gfx_surface_state::ready_to_render or not redraw_rectangle) {
        return r;
    }

    ttlet optional_frame_buffer_index = acquireNextImageFromSwapchain();
    if (!optional_frame_buffer_index) {
        // No image is ready to be rendered, yet, possibly because our vertical sync function
        // is not working correctly.
        return r;
    }

    // Setting the frame buffer index, also enabled the draw_context.
    r.frame_buffer_index = narrow<size_t>(*optional_frame_buffer_index);

    // Record which part of the image will be redrawn on the current swapchain image.
    auto &current_image = swapchain_image_infos.at(r.frame_buffer_index);
    current_image.redraw_rectangle = redraw_rectangle;

    // Calculate the scissor rectangle, from the combined redraws of the complete swapchain.
    // We need to do this so that old redraws are also executed in the current swapchain image.
    r.scissor_rectangle = ceil(
        std::accumulate(swapchain_image_infos.cbegin(), swapchain_image_infos.cend(), aarectangle{}, [](ttlet &sum, ttlet &item) {
            return sum | item.redraw_rectangle;
        }));

    // Wait until previous rendering has finished, before the next rendering.
    vulkan_device().waitForFences({renderFinishedFence}, VK_TRUE, std::numeric_limits<uint64_t>::max());

    // Unsignal the fence so we will not modify/destroy the command buffers during rendering.
    vulkan_device().resetFences({renderFinishedFence});

    return r;
}

void gfx_surface_vulkan::render_finish(draw_context const &context)
{
    ttlet lock = std::scoped_lock(gfx_system_mutex);

    auto &current_image = swapchain_image_infos.at(context.frame_buffer_index);

    fill_command_buffer(current_image, context);
    submitCommandBuffer();

    // Signal the fence when all rendering has finished on the graphics queue.
    // When the fence is signaled we can modify/destroy the command buffers.
    [[maybe_unused]] ttlet submit_result = _graphics_queue->queue.submit(0, nullptr, renderFinishedFence);

    presentImageToQueue(narrow_cast<uint32_t>(context.frame_buffer_index), renderFinishedSemaphore);

    // Do an early tear down of invalid vulkan objects.
    teardown();
}

void gfx_surface_vulkan::fill_command_buffer(swapchain_image_info &current_image, draw_context const &context)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    auto t = trace<"fill_command_buffer">{};

    commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    ttlet background_color_f32x4 = static_cast<f32x4>(context.background_color);
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
    ttlet scissor_rectangle = ceil(intersect(
        context.scissor_rectangle,
        aarectangle{
            0.0f, 0.0f, narrow_cast<float>(swapchainImageExtent.width), narrow_cast<float>(swapchainImageExtent.height)}));

    ttlet scissors = std::array{vk::Rect2D{
        vk::Offset2D(
            narrow_cast<uint32_t>(scissor_rectangle.left()),
            narrow_cast<uint32_t>(swapchainImageExtent.height - scissor_rectangle.bottom() - scissor_rectangle.height())),
        vk::Extent2D(narrow_cast<uint32_t>(scissor_rectangle.width()), narrow_cast<uint32_t>(scissor_rectangle.height()))}};

    // The scissor and render area makes sure that the frame buffer is not modified where we are not drawing the widgets.
    commandBuffer.setScissor(0, scissors);

    ttlet render_area = scissors.at(0);

    // Because we use a scissor the image from the swapchain around the scissor-area is reused.
    // Because of reuse the swapchain image must already be in the "ePresentSrcKHR" layout.
    // The swapchain creates images in undefined layout, so we need to change the layout once.
    if (not current_image.layout_is_present) {
        gfx_device_vulkan::transition_layout(
            commandBuffer,
            current_image.image,
            swapchainImageFormat.format,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR);

        current_image.layout_is_present = true;
    }

    commandBuffer.beginRenderPass(
        {renderPass, current_image.frame_buffer, render_area, narrow_cast<uint32_t>(clearValues.size()), clearValues.data()},
        vk::SubpassContents::eInline);

    boxPipeline->drawInCommandBuffer(commandBuffer, context);
    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    imagePipeline->drawInCommandBuffer(commandBuffer, context);
    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    SDFPipeline->drawInCommandBuffer(commandBuffer, context);
    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    toneMapperPipeline->drawInCommandBuffer(commandBuffer, context);

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

    _graphics_queue->queue.submit(submitInfo, vk::Fence());
}

std::tuple<std::size_t, extent2> gfx_surface_vulkan::get_image_count_and_size(std::size_t new_count, extent2 new_size)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    ttlet surfaceCapabilities = vulkan_device().getSurfaceCapabilitiesKHR(intrinsic);

    ttlet min_count = narrow_cast<std::size_t>(surfaceCapabilities.minImageCount);
    ttlet max_count = narrow_cast<std::size_t>(surfaceCapabilities.maxImageCount ? surfaceCapabilities.maxImageCount : 3);
    ttlet clamped_count = std::clamp(new_count, min_count, max_count);
    tt_log_info(
        "gfx_surface min_count={}, max_count={}, requested_count={}, count={}", min_count, max_count, new_count, clamped_count);

    // minImageExtent and maxImageExtent are always valid. currentImageExtent may be 0xffffffff.
    ttlet min_size = extent2{
        narrow_cast<float>(surfaceCapabilities.minImageExtent.width),
        narrow_cast<float>(surfaceCapabilities.minImageExtent.height)};
    ttlet max_size = extent2{
        narrow_cast<float>(surfaceCapabilities.maxImageExtent.width),
        narrow_cast<float>(surfaceCapabilities.maxImageExtent.height)};
    ttlet clamped_size = clamp(new_size, min_size, max_size);

    tt_log_info("gfx_surface min_size={}, max_size={}, requested_size={}, size={}", min_size, max_size, new_size, clamped_size);
    return {clamped_count, clamped_size};
}

void gfx_surface_vulkan::buildDevice()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());
}

bool gfx_surface_vulkan::buildSurface()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    return vulkan_device().score(*this) > 0;
}

gfx_surface_state gfx_surface_vulkan::buildSwapchain(std::size_t new_count, extent2 new_size)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    tt_log_info("Building swap chain");

    ttlet sharingMode = _graphics_queue == _present_queue ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;

    std::array<uint32_t, 2> const sharingQueueFamilyAllIndices = {
        _graphics_queue->family_queue_index, _present_queue->family_queue_index};

    swapchainImageFormat = vulkan_device().get_surface_format(*this);
    nrSwapchainImages = narrow_cast<uint32_t>(new_count);
    swapchainImageExtent = VkExtent2D{narrow_cast<uint32_t>(new_size.width()), narrow_cast<uint32_t>(new_size.height())};
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
        sharingMode == vk::SharingMode::eConcurrent ? narrow_cast<uint32_t>(sharingQueueFamilyAllIndices.size()) : 0,
        sharingMode == vk::SharingMode::eConcurrent ? sharingQueueFamilyAllIndices.data() : nullptr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vulkan_device().get_present_mode(*this),
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
    depthAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    depthAllocationCreateInfo.pUserData = const_cast<char *>("vk::Image depth attachment");
    depthAllocationCreateInfo.usage = vulkan_device().lazyMemoryUsage;
    std::tie(depthImage, depthImageAllocation) = vulkan_device().createImage(depthImageCreateInfo, depthAllocationCreateInfo);
    vulkan_device().setDebugUtilsObjectNameEXT(depthImage, "vk::Image depth attachment");

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
    colorAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    colorAllocationCreateInfo.pUserData = const_cast<char *>("vk::Image color attachment");
    colorAllocationCreateInfo.usage = vulkan_device().lazyMemoryUsage;

    std::tie(colorImages[0], colorImageAllocations[0]) =
        vulkan_device().createImage(colorImageCreateInfo, colorAllocationCreateInfo);
    vulkan_device().setDebugUtilsObjectNameEXT(colorImages[0], "vk::Image color attachment");

    return gfx_surface_state::ready_to_render;
}

void gfx_surface_vulkan::teardownSwapchain()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    vulkan_device().destroy(swapchain);
    vulkan_device().destroyImage(depthImage, depthImageAllocation);

    for (std::size_t i = 0; i != colorImages.size(); ++i) {
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

    for (std::size_t i = 0; i != colorImageViews.size(); ++i) {
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

        ttlet attachments = std::array{depthImageView, colorImageViews[0], image_view};

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
    for (std::size_t i = 0; i != colorImageViews.size(); ++i) {
        vulkan_device().destroy(colorImageViews[i]);
    }
}

/** Build render passes.
 *
 * One pass, with 4 subpasses:
 *  1. box shader: to color-attachment+depth
 *  2. image shader: to color-attachmen+depth
 *  3. sdf shader: to color-attachmne+depth
 *  4. tone-mapper: color-input-attachment to swapchain-attachment.
 *
 * Rendering is done on a float-16 RGBA color-attachment.
 * In the last subpass the color-attachment is translated to the swap-chain attachment.
 */
void gfx_surface_vulkan::buildRenderPasses()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    ttlet attachment_descriptions = std::array{
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

    ttlet depth_attachment_reference = vk::AttachmentReference{0, vk::ImageLayout::eDepthStencilAttachmentOptimal};
    ttlet color_attachment_references = std::array{vk::AttachmentReference{1, vk::ImageLayout::eColorAttachmentOptimal}};
    ttlet color_input_attachment_references = std::array{vk::AttachmentReference{1, vk::ImageLayout::eShaderReadOnlyOptimal}};
    ttlet swapchain_attachment_references = std::array{vk::AttachmentReference{2, vk::ImageLayout::eColorAttachmentOptimal}};

    ttlet subpass_descriptions = std::array{
        vk::SubpassDescription{// Subpass 0 Box
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               0, // inputAttchmentReferencesCount
                               nullptr, // inputAttachmentReferences
                               narrow_cast<uint32_t>(color_attachment_references.size()),
                               color_attachment_references.data(),
                               nullptr, // resolveAttachments
                               &depth_attachment_reference

        },
        vk::SubpassDescription{// Subpass 1 Image
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               0, // inputAttchmentReferencesCount
                               nullptr, // inputAttachmentReferences
                               narrow_cast<uint32_t>(color_attachment_references.size()),
                               color_attachment_references.data(),
                               nullptr, // resolveAttachments
                               &depth_attachment_reference

        },
        vk::SubpassDescription{// Subpass 2 SDF
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               0,
                               nullptr,
                               narrow_cast<uint32_t>(color_attachment_references.size()),
                               color_attachment_references.data(),
                               nullptr, // resolveAttachments
                               &depth_attachment_reference

        },
        vk::SubpassDescription{// Subpass 3 tone-mapper
                               vk::SubpassDescriptionFlags(),
                               vk::PipelineBindPoint::eGraphics,
                               narrow_cast<uint32_t>(color_input_attachment_references.size()),
                               color_input_attachment_references.data(),
                               narrow_cast<uint32_t>(swapchain_attachment_references.size()),
                               swapchain_attachment_references.data(),
                               nullptr,
                               nullptr}};

    ttlet subpass_dependency = std::array{
        vk::SubpassDependency{
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eMemoryRead,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 0: Render shaded polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            0,
            1,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentRead,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 1: Render texture mapped polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            1,
            2,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentRead,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 2: Render SDF-texture mapped polygons to color+depth with fixed function alpha compositing
        vk::SubpassDependency{
            2,
            3,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead,
            vk::DependencyFlagBits::eByRegion},
        // Subpass 3: Tone mapping color to swapchain.
        vk::SubpassDependency{
            3,
            VK_SUBPASS_EXTERNAL,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eMemoryRead,
            vk::DependencyFlagBits::eByRegion}};

    vk::RenderPassCreateInfo const render_pass_create_info = {
        vk::RenderPassCreateFlags(),
        narrow_cast<uint32_t>(attachment_descriptions.size()), // attachmentCount
        attachment_descriptions.data(), // attachments
        narrow_cast<uint32_t>(subpass_descriptions.size()), // subpassCount
        subpass_descriptions.data(), // subpasses
        narrow_cast<uint32_t>(subpass_dependency.size()), // dependencyCount
        subpass_dependency.data() // dependencies
    };

    renderPass = vulkan_device().createRenderPass(render_pass_create_info);
    ttlet granularity = vulkan_device().getRenderAreaGranularity(renderPass);
    _render_area_granularity = extent2{narrow<float>(granularity.width), narrow<float>(granularity.height)};
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
        vulkan_device().allocateCommandBuffers({_graphics_queue->command_pool, vk::CommandBufferLevel::ePrimary, 1});

    commandBuffer = commandBuffers.at(0);
}

void gfx_surface_vulkan::teardownCommandBuffers()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());
    ttlet commandBuffers = std::vector<vk::CommandBuffer>{commandBuffer};

    vulkan_device().freeCommandBuffers(_graphics_queue->command_pool, commandBuffers);
}

void gfx_surface_vulkan::teardownSurface()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    down_cast<gfx_system_vulkan &>(system).destroySurfaceKHR(intrinsic);
}

void gfx_surface_vulkan::teardownDevice()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    _device = nullptr;
}

} // namespace tt::inline v1
