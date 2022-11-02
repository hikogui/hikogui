// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/window_widget.hpp Defines window_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "../geometry/axis.hpp"
#include "../label.hpp"

namespace hi { inline namespace v1 {
class toolbar_widget;
class grid_widget;

template<typename Context>
concept window_widget_attribute = forward_of<Context, observer<hi::label>>;

/** The top-level window widget.
 * This widget is the top-level widget that is owned by the `gui_window`.
 * It contains as childs the toolbar and content `grid_widget`.
 *
 * @ingroup widgets
 */
class window_widget final : public widget {
public:
    using super = widget;

    observer<label> title;

    window_widget(gui_window *window, window_widget_attribute auto&&...attributes) noexcept : window_widget(window)
    {
        set_attributes(hi_forward(attributes)...);
    }

    /** The background color of the window.
     * This function is used during rendering to use the optimized
     * GPU clear function.
     */
    [[nodiscard]] color background_color() noexcept;

    /** Get a reference to the window's content widget.
     * @see grid_widget
     * @return A reference to a grid_widget.
     */
    [[nodiscard]] grid_widget& content() noexcept;

    /** Get a reference to window's toolbar widget.
     * @see toolbar_widget
     * @return A reference to a toolbar_widget.
     */
    [[nodiscard]] toolbar_widget& toolbar() noexcept;

    [[nodiscard]] axis resize_axis() const noexcept
    {
        auto r = axis::none;
        if (_constraints.minimum.width() != _constraints.maximum.width()) {
            r |= axis::width;
        }
        if (_constraints.minimum.height() != _constraints.maximum.height()) {
            r |= axis::height;
        }
        return r;
    }

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override;
    widget_constraints const& set_constraints(set_constraints_context const& context) noexcept override;
    void set_layout(widget_layout const& context) noexcept;
    void draw(draw_context const& context) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override;
    bool handle_event(gui_event const& event) noexcept override;
    bool process_event(gui_event const& event) const noexcept override;
    /// @endprivatesection
private:
    gui_window *_window;

    aarectangle _content_rectangle;
    std::unique_ptr<grid_widget> _content;

    aarectangle _toolbar_rectangle;
    std::unique_ptr<toolbar_widget> _toolbar;

    window_widget(gui_window *window) noexcept;

    void set_attributes() noexcept {}
    void set_attributes(window_widget_attribute auto&& first, window_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::label>>) {
            title = hi_forward(first);
        } else {
            hi_static_no_default();
        }

        set_attributes(hi_forward(rest)...);
    }
};

}} // namespace hi::v1
