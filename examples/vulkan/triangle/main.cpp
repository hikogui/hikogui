// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/codec/png.hpp"
#include "hikogui/GUI/gui_system.hpp"
#include "hikogui/GFX/RenderDoc.hpp"
#include "hikogui/widgets/vulkan_widget.hpp"
#include "hikogui/crt.hpp"
#include "hikogui/loop.hpp"
#include "hikogui/task.hpp"
#include "hikogui/ranges.hpp"
#include "triangle.hpp"
#include <ranges>
#include <cassert>

// Every widget must inherit from hi::widget.
class triangle_widget : public hi::vulkan_widget {
public:
    // Every constructor of a widget starts with a `window` and `parent` argument.
    // In most cases these are automatically filled in when calling a container widget's `make_widget()` function.
    triangle_widget(hi::gui_window& window, hi::widget *parent) noexcept : vulkan_widget(window, parent) {}

    // The set_constraints() function is called when the window is first initialized,
    // or when a widget wants to change its constraints.
    hi::widget_constraints const& set_constraints() noexcept override
    {
        // Almost all widgets will reset the `_layout` variable here so that it will
        // trigger the calculations in `set_layout()` as well.
        _layout = {};

        // Certain expensive calculations, such as loading of images and shaping of text
        // can be done in this function.

        // The constrains below have different minimum, preferred and maximum sizes.
        // When the window is initially created it will try to size itself so that
        // the contained widgets are at their preferred size. Having a different minimum
        // and/or maximum size will allow the window to be resizable.
        return _constraints = {{400, 300}, {640, 480}, {1024, 860}, theme().margin};
    }

    // The `set_layout()` function is called when the window has resized, or when
    // a widget wants to change the internal layout.
    //
    // NOTE: The size of the layout may be larger than the maximum constraints of this widget.
    void set_layout(hi::widget_layout const& layout) noexcept override
    {
        // Update the `_layout` with the new context, in this case we want to do some
        // calculations when the size of the widget was changed.
        if (compare_store(_layout, layout)) {
            auto view_port = _layout.window_rectangle();
            auto window_height = window.widget->layout().height();

            _view_port = VkRect2D{
                VkOffset2D{hi::narrow_cast<int32_t>(view_port.left()), hi::narrow_cast<int32_t>(window_height - view_port.top())},
                VkExtent2D{hi::narrow_cast<uint32_t>(view_port.width()), hi::narrow_cast<uint32_t>(view_port.height())}};
        }
    }

    void build_for_new_device(
        VmaAllocator allocator,
        vk::Instance instance,
        vk::Device device,
        vk::Queue graphics_queue,
        uint32_t graphics_queue_family_index) noexcept override
    {
        _triangle_example = std::make_shared<TriangleExample>(
            allocator, static_cast<VkDevice>(device), static_cast<VkQueue>(graphics_queue), graphics_queue_family_index);
    }

    void build_for_new_swapchain(std::vector<vk::ImageView> const& views, vk::Extent2D size, vk::SurfaceFormatKHR format) noexcept
        override
    {
        auto views_ = hi::make_vector<VkImageView>(std::views::transform(views, [](auto const& view) {
            return static_cast<VkImageView>(view);
        }));

        assert(_triangle_example != nullptr);
        _triangle_example->buildForNewSwapchain(views_, static_cast<VkExtent2D>(size), static_cast<VkFormat>(format.format));
    }

    void draw(uint32_t swapchain_index, vk::Semaphore start, vk::Semaphore finish, vk::Rect2D render_area) noexcept override
    {
        auto x_offset = render_area.offset.x;

        assert(_triangle_example != nullptr);
        _triangle_example->render(
            swapchain_index,
            static_cast<VkSemaphore>(start),
            static_cast<VkSemaphore>(finish),
            static_cast<VkRect2D>(render_area),
            _view_port);
    }

    void teardown_for_device_lost() noexcept override
    {
        _triangle_example = {};
    }

    void teardown_for_window_lost() noexcept override
    {
        _triangle_example = {};
    }

    void teardown_for_swapchain_lost() noexcept override
    {
        assert(_triangle_example != nullptr);
        _triangle_example->teardownForLostSwapchain();
    }

private:
    std::shared_ptr<TriangleExample> _triangle_example;
    VkRect2D _view_port;
};

hi::task<> main_window(hi::gui_system& gui)
{
    auto icon = hi::icon(hi::png::load(hi::URL{"resource:vulkan_triangle.png"}));
    auto window = gui.make_window(hi::label{std::move(icon), hi::tr("Vulkan Triangle")});
    window->content().make_widget<triangle_widget>("A1");

    co_await window->closing;
}

int hi_main(int argc, char *argv[])
{
    auto doc = hi::RenderDoc{};

    auto gui = hi::gui_system::make_unique();
    main_window(*gui);
    return hi::loop::main().resume();
}
