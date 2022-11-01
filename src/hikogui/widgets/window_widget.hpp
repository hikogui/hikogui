// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/window_widget.hpp Defines window_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "../label.hpp"

namespace hi { inline namespace v1 {
class toolbar_widget;
class system_menu_widget;
class grid_widget;

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

    window_widget(gui_window *window, forward_of<observer<label>> auto&& title) noexcept :
        super(nullptr), _window(window), title(hi_forward(title))
    {
        hi_assert_not_null(_window);
        constructor_implementation();
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
    widget_constraints _content_constraints;
    std::unique_ptr<grid_widget> _content;

    aarectangle _toolbar_rectangle;
    widget_constraints _toolbar_constraints;
    std::unique_ptr<toolbar_widget> _toolbar;
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
    system_menu_widget *_system_menu = nullptr;
#endif

    void constructor_implementation() noexcept;
};

}} // namespace hi::v1
