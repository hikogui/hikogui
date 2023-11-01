// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include <optional>

export module hikogui_GFX : gfx_surface_intf;
import : gfx_device_intf;
import : gfx_pipeline_SDF_intf;
import : gfx_pipeline_box_intf;
import : gfx_pipeline_image_intf;
import : gfx_pipeline_override_intf;
import : gfx_pipeline_tone_mapper_intf;
import : gfx_queue;
import : gfx_surface_delegate;
import : gfx_surface_state;

export namespace hi::inline v1 {

struct swapchain_image_info {
    vk::Image image;
    vk::ImageView image_view;
    vk::Framebuffer frame_buffer;
    aarectangle redraw_rectangle;
    bool layout_is_present = false;
};

class gfx_surface {
public:
    gfx_surface_state state = gfx_surface_state::has_window;
    gfx_surface_loss loss = gfx_surface_loss::none;

    vk::SurfaceKHR intrinsic;

    vk::SwapchainKHR swapchain;

    constexpr static uint32_t defaultNumberOfSwapchainImages = 2;

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

    std::unique_ptr<gfx_pipeline_image> image_pipeline;
    std::unique_ptr<gfx_pipeline_box> box_pipeline;
    std::unique_ptr<gfx_pipeline_SDF> SDF_pipeline;
    std::unique_ptr<gfx_pipeline_override> override_pipeline;
    std::unique_ptr<gfx_pipeline_tone_mapper> tone_mapper_pipeline;

    gfx_surface(vk::SurfaceKHR surface) : intrinsic(surface)
    {
        box_pipeline = std::make_unique<gfx_pipeline_box>(this);
        image_pipeline = std::make_unique<gfx_pipeline_image>(this);
        SDF_pipeline = std::make_unique<gfx_pipeline_SDF>(this);
        override_pipeline = std::make_unique<gfx_pipeline_override>(this);
        tone_mapper_pipeline = std::make_unique<gfx_pipeline_tone_mapper>(this);
    }

    ~gfx_surface()
    {
        if (state != gfx_surface_state::no_window) {
            hilet lock = std::scoped_lock(gfx_system_mutex);
            loss = gfx_surface_loss::window_lost;
            teardown();
            hi_assert(state == gfx_surface_state::no_window);
        }
    }

    gfx_surface(const gfx_surface&) = delete;
    gfx_surface& operator=(const gfx_surface&) = delete;
    gfx_surface(gfx_surface&&) = delete;
    gfx_surface& operator=(gfx_surface&&) = delete;

    /*! Set GPU device to manage this window.
     * Change of the device may be done at runtime.
     *
     * @param new_device The device to use for rendering, may be nullptr.
     */
    void set_device(gfx_device *device) noexcept;

    [[nodiscard]] gfx_device *device() const noexcept
    {
        return _device;
    }

    [[nodiscard]] extent2 size() const noexcept;

    void update(extent2 new_size) noexcept;

    [[nodiscard]] draw_context render_start(aarectangle redraw_rectangle);
    void render_finish(draw_context const& context);

    void add_delegate(gfx_surface_delegate *delegate) noexcept;
    void remove_delegate(gfx_surface_delegate *delegate) noexcept;

private:
    struct delegate_type {
        gfx_surface_delegate *delegate;
        vk::Semaphore semaphore;
    };

    gfx_device *_device = nullptr;

    std::vector<delegate_type> _delegates;

    gfx_queue_vulkan const *_graphics_queue;
    gfx_queue_vulkan const *_present_queue;
    extent2 _render_area_granularity;

    void teardown() noexcept;
    void build(extent2 new_size) noexcept;

    gfx_surface_loss build_for_new_device() noexcept;
    gfx_surface_loss build_for_new_swapchain(extent2 new_size) noexcept;

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

    bool read_surface_extent(extent2 minimum_size, extent2 maximum_size);
    bool check_surface_extent();

    void build_semaphores();
    void teardown_semaphores();
    gfx_surface_loss build_swapchain(std::size_t new_count, extent2 new_size);
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
    std::tuple<std::size_t, extent2> get_image_count_and_size(std::size_t new_count, extent2 new_size);
};

[[nodiscard]] std::unique_ptr<gfx_surface> make_unique_gfx_surface(os_handle instance, void *os_window);

[[nodiscard]] gfx_device *find_best_device(gfx_surface const &surface)
{
    return find_best_device(surface.intrinsic);
}

} // namespace hi::inline v1
