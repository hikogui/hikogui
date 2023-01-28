// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_surface.hpp"
#include "gfx_queue_vulkan.hpp"
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include <optional>

namespace hi::inline v1 {
class gfx_surface_delegate_vulkan;
class gfx_device_vulkan;
namespace pipeline_image {
class pipeline_image;
}
namespace pipeline_box {
class pipeline_box;
}
namespace pipeline_SDF {
class pipeline_SDF;
}
namespace pipeline_alpha {
class pipeline_alpha;
}
namespace pipeline_tone_mapper {
class pipeline_tone_mapper;
}

struct swapchain_image_info {
    vk::Image image;
    vk::ImageView image_view;
    vk::Framebuffer frame_buffer;
    aarectanglei redraw_rectangle;
    bool layout_is_present = false;
};

class gfx_surface_vulkan final : public gfx_surface {
public:
    using super = gfx_surface;

    vk::SurfaceKHR intrinsic;

    vk::SwapchainKHR swapchain;

    static constexpr uint32_t defaultNumberOfSwapchainImages = 2;

    uint32_t nrSwapchainImages;
    vk::Extent2D swapchainImageExtent;
    vk::SurfaceFormatKHR swapchainImageFormat;
    std::vector<swapchain_image_info> swapchain_image_infos;

    // static const vk::Format depthImageFormat = vk::Format::eD32Sfloat;
    static const vk::Format depthImageFormat = vk::Format::eD16Unorm;
    VmaAllocation depthImageAllocation;
    vk::Image depthImage;
    vk::ImageView depthImageView;

    static const vk::Format colorImageFormat = vk::Format::eR16G16B16A16Sfloat;
    std::array<VmaAllocation, 1> colorImageAllocations;
    std::array<vk::Image, 1> colorImages;
    std::array<vk::ImageView, 1> colorImageViews;
    std::array<vk::DescriptorImageInfo, 1> colorDescriptorImageInfos;

    vk::RenderPass renderPass;

    vk::CommandBuffer commandBuffer;

    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;
    vk::Fence renderFinishedFence;

    std::unique_ptr<pipeline_image::pipeline_image> image_pipeline;
    std::unique_ptr<pipeline_box::pipeline_box> box_pipeline;
    std::unique_ptr<pipeline_SDF::pipeline_SDF> SDF_pipeline;
    std::unique_ptr<pipeline_alpha::pipeline_alpha> alpha_pipeline;
    std::unique_ptr<pipeline_tone_mapper::pipeline_tone_mapper> tone_mapper_pipeline;

    gfx_surface_vulkan(gfx_system& system, vk::SurfaceKHR surface);
    ~gfx_surface_vulkan();

    gfx_surface_vulkan(const gfx_surface_vulkan&) = delete;
    gfx_surface_vulkan& operator=(const gfx_surface_vulkan&) = delete;
    gfx_surface_vulkan(gfx_surface_vulkan&&) = delete;
    gfx_surface_vulkan& operator=(gfx_surface_vulkan&&) = delete;

    void init() override;

    void set_device(gfx_device *device) noexcept override;

    gfx_device_vulkan& vulkan_device() const noexcept;
    [[nodiscard]] extent2i size() const noexcept override;

    void update(extent2i new_size) noexcept override;

    [[nodiscard]] draw_context render_start(aarectanglei redraw_rectangle) override;
    void render_finish(draw_context const& context) override;

    void add_delegate(gfx_surface_delegate *delegate) noexcept override;
    void remove_delegate(gfx_surface_delegate *delegate) noexcept override;

protected:
    void teardown() noexcept override;
    void build(extent2i new_size) noexcept;

private:
    struct delegate_type {
        gfx_surface_delegate_vulkan *delegate;
        vk::Semaphore semaphore;
    };

    std::vector<delegate_type> _delegates;

    gfx_queue_vulkan const *_graphics_queue;
    gfx_queue_vulkan const *_present_queue;
    extent2i _render_area_granularity;

    gfx_surface_loss build_for_new_device() noexcept;
    gfx_surface_loss build_for_new_swapchain(extent2i new_size) noexcept;

    void teardown_for_swapchain_lost() noexcept;
    void teardown_for_device_lost() noexcept;
    void teardown_for_window_lost() noexcept;

    std::optional<uint32_t> acquire_next_image_from_swapchain();
    void present_image_to_queue(uint32_t frameBufferIndex, vk::Semaphore renderFinishedSemaphore);

    /**
     * @param current_image Information about the swapchain-image to be rendered.
     * @param context The drawing context.
     */
    void fill_command_buffer(swapchain_image_info const& current_image, draw_context const& context, vk::Rect2D render_area);

    /** Submit the command buffer updated with fill command buffer.
     *
     * @param delegate_semaphore The semaphore of the last delegate to trigger writing into the swapchain-image.
     */
    void submit_command_buffer(vk::Semaphore delegate_semaphore);

    bool read_surface_extent(extent2i minimum_size, extent2i maximum_size);
    bool check_surface_extent();

    void build_semaphores();
    void teardown_semaphores();
    gfx_surface_loss build_swapchain(std::size_t new_count, extent2i new_size);
    void teardown_swapchain();
    void build_command_buffers();
    void teardown_command_buffers();
    void build_render_passes();
    void teardown_render_passes();
    void build_frame_buffers();
    void teardown_frame_buffers();
    void build_pipelines();
    void teardown_pipelines();

    void wait_idle();

    /** Get the image size and image count from the Vulkan surface.
     *
     * This function will return an appropriate
     *
     * @param new_count Request the number of images in the swapchain.
     * @param new_size Request the image size in the swapchain.
     * @return A valid swapchain image count, swapchain image size.
     */
    std::tuple<std::size_t, extent2i> get_image_count_and_size(std::size_t new_count, extent2i new_size);
};

} // namespace hi::inline v1
