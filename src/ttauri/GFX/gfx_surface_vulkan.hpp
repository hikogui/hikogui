// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_surface.hpp"
#include "gfx_queue_vulkan.hpp"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <optional>

namespace tt::inline v1 {
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
namespace pipeline_tone_mapper {
class pipeline_tone_mapper;
}

struct swapchain_image_info {
    vk::Image image;
    vk::ImageView image_view;
    vk::Framebuffer frame_buffer;
    aarectangle redraw_rectangle;
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

    std::unique_ptr<pipeline_image::pipeline_image> imagePipeline;
    std::unique_ptr<pipeline_box::pipeline_box> boxPipeline;
    std::unique_ptr<pipeline_SDF::pipeline_SDF> SDFPipeline;
    std::unique_ptr<pipeline_tone_mapper::pipeline_tone_mapper> toneMapperPipeline;

    gfx_surface_vulkan(gfx_system &system, vk::SurfaceKHR surface);
    ~gfx_surface_vulkan();

    gfx_surface_vulkan(const gfx_surface_vulkan &) = delete;
    gfx_surface_vulkan &operator=(const gfx_surface_vulkan &) = delete;
    gfx_surface_vulkan(gfx_surface_vulkan &&) = delete;
    gfx_surface_vulkan &operator=(gfx_surface_vulkan &&) = delete;

    void init() override;

    void set_device(gfx_device *device) noexcept override;

    gfx_device_vulkan &vulkan_device() const noexcept;
    [[nodiscard]] extent2 size() const noexcept override;

    void update(extent2 new_size) noexcept override;

    [[nodiscard]] draw_context render_start(aarectangle redraw_rectangle) override;
    void render_finish(draw_context const &context) override;

protected:
    void teardown() override;

private:
    gfx_queue_vulkan const *_graphics_queue;
    gfx_queue_vulkan const *_present_queue;
    extent2 _render_area_granularity;

    void build(extent2 new_size);

    std::optional<uint32_t> acquireNextImageFromSwapchain();
    void presentImageToQueue(uint32_t frameBufferIndex, vk::Semaphore renderFinishedSemaphore);

    void fill_command_buffer(swapchain_image_info &current_image, draw_context const &context);
    void submitCommandBuffer();

    bool readSurfaceExtent(extent2 minimum_size, extent2 maximum_size);
    bool checkSurfaceExtent();

    void buildDevice();
    void buildSemaphores();
    void teardownSemaphores();
    gfx_surface_state buildSwapchain(std::size_t new_count, extent2 new_size);
    void teardownSwapchain();
    void buildCommandBuffers();
    void teardownCommandBuffers();
    void buildRenderPasses();
    void teardownRenderPasses();
    void buildFramebuffers();
    void teardownFramebuffers();
    void buildPipelines();
    void teardownPipelines();
    bool buildSurface();
    void teardownSurface();
    void teardownDevice();

    void waitIdle();

    /** Get the image size and image count from the Vulkan surface.
     *
     * This function will return an appropriate
     *
     * @param new_count Request the number of images in the swapchain.
     * @param new_size Request the image size in the swapchain.
     * @return A valid swapchain image count, swapchain image size.
     */
    std::tuple<std::size_t, extent2> get_image_count_and_size(std::size_t new_count, extent2 new_size);
};

} // namespace tt::inline v1
