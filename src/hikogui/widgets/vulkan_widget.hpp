// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../GUI/gui_system.hpp"
#include "widget.hpp"
#include <vulkan/vulkan.hpp>

namespace hi::inline v1 {

/** A widget that draws directly into the swap-chain.
 */
class vulkan_widget : public widget {
public:
    using super = widget;

    vulkan_widget(hi::gui_window& window, hi::widget *parent) noexcept : super(window, parent) {}

    widget_constraints const& set_constraints() noexcept override
    {
        _layout = {};
        return _constraints = {{100, 50}, {200, 100}, {300, 100}, theme().margin};
    }

    void set_layout(hi::widget_layout const& layout) noexcept override
    {
        if (compare_store(_layout, layout)) {}
    }

    /** Draw using vulkan API.
     *
     * @param view The swap-chain's image view to draw in.
     * @param size The size of the frame buffer
     * @param format The pixel format
     * @param clipping_rectangle The rectangle on the window that is visible through this widget.
     * @param render_area The area of the window that is being drawn.
     * @param start The semaphore used to signal when the @a image_view is ready to be drawn.
     * @param finish The semaphore used to signal when the HikoGUI overlay is drawn onto the @a image_view.
     */
    virtual void draw_vulkan(vk::ImageView view, vk::Extent2 size, vk::Format format, aarectangle clipping_rectangle, aarectangle render_area, vk::Semaphore start, vk::Semaphore finish) noexcept {}

    void draw(hi::draw_context const& context) noexcept override
    {
        if (*visible and overlaps(context, layout())) {
            context.make_hole(_layout, _layout.rectangle());

            draw_vulkan(aarectangle{_layout.to_window * _layout.rectangle()}, context.scissor_rectangle);
        }
    }
};

} // namespace hi::inline v1
