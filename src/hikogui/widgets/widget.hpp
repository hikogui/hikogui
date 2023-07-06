// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/widget.hpp Defines widget.
 * @ingroup widgets
 */

#pragma once

#include "widget_mode.hpp"
#include "../layout/module.hpp"
#include "../geometry/module.hpp"
#include "../observer/module.hpp"
#include "../time/module.hpp"
#include "../settings/module.hpp"
#include "../numeric/module.hpp"
#include "../GUI/module.hpp"
#include "../generator.hpp"
#include <memory>
#include <vector>
#include <string>
#include <ranges>

namespace hi { inline namespace v1 {

/** An interactive graphical object as part of the user-interface.
 *
 * Rendering is done in three distinct phases:
 *  1. Updating Constraints: `widget::update_constraints()`
 *  2. Updating Layout: `widget::set_layout()`
 *  3. Drawing: `widget::draw()`
 *
 * @ingroup widgets
 */
class widget : public widget_intf {
public:
    /** The widget mode.
     * The current visibility and interactivity of a widget.
     */
    observer<widget_mode> mode = widget_mode::enabled;

    /** Mouse cursor is hovering over the widget.
     */
    observer<bool> hover = false;

    /** The widget has keyboard focus.
     */
    observer<bool> focus = false;

    /** The draw layer of the widget.
     * The semantic layer is used mostly by the `draw()` function
     * for selecting colors from the theme, to denote nesting widgets
     * inside other widgets.
     *
     * Semantic layers start at 0 for the window-widget and for any pop-up
     * widgets.
     *
     * The semantic layer is increased by one, whenever a user of the
     * user-interface would understand the next layer to begin.
     *
     * In most cases it would mean that a container widget that does not
     * draw itself will not increase the semantic_layer number.
     */
    int semantic_layer = 0;

    /** The logical layer of the widget.
     * The logical layer can be used to determine how far away
     * from the window-widget (root) the current widget is.
     *
     * Logical layers start at 0 for the window-widget.
     * Each child widget increases the logical layer by 1.
     */
    int logical_layer = 0;

    /** The minimum size this widget is allowed to be.
     */
    observer<extent2> minimum = extent2{};

    /** The maximum size this widget is allowed to be.
     */
    observer<extent2> maximum = extent2::large();

    /*! Constructor for creating sub views.
     */
    explicit widget(widget *parent) noexcept;

    virtual ~widget();
    widget(const widget&) = delete;
    widget& operator=(const widget&) = delete;
    widget(widget&&) = delete;
    widget& operator=(widget&&) = delete;

    using widget_intf::children;
    [[nodiscard]] generator<widget_intf &> children(bool include_invisible) noexcept override
    {
        co_return;
    }

    /** Find the widget that is under the mouse cursor.
     * This function will recursively test with visual child widgets, when
     * widgets overlap on the screen the hitbox object with the highest elevation is returned.
     *
     * @param position The coordinate of the mouse local to the widget.
     * @return A hit_box object with the cursor-type and a reference to the widget.
     */
    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        return {};
    }

    /** Call hitbox_test from a parent widget.
     *
     * This function will transform the position from parent coordinates to local coordinates.
     *
     * @param position The coordinate of the mouse local to the parent widget.
     */
    [[nodiscard]] virtual hitbox hitbox_test_from_parent(point2 position) const noexcept
    {
        return hitbox_test(_layout.from_parent * position);
    }

    /** Call hitbox_test from a parent widget.
     *
     * This function will transform the position from parent coordinates to local coordinates.
     *
     * @param position The coordinate of the mouse local to the parent widget.
     * @param sibling_hitbox The hitbox of a sibling to combine with the hitbox of this widget.
     */
    [[nodiscard]] virtual hitbox hitbox_test_from_parent(point2 position, hitbox sibling_hitbox) const noexcept
    {
        return std::max(sibling_hitbox, hitbox_test(_layout.from_parent * position));
    }

    /** Check if the widget will accept keyboard focus.
     *
     */
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        return false;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _layout = {};
        return {*minimum, *minimum, *maximum};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        _layout = context;
    }

    widget_layout const& layout() const noexcept override
    {
        return _layout;
    }

    void draw(draw_context const& context) noexcept override {}

    /** Send a event to the window.
     */
    bool process_event(gui_event const& event) const noexcept override
    {
        if (parent != nullptr) {
            return parent->process_event(event);
        } else {
            return true;
        }
    }

    /** Request the widget to be redrawn on the next frame.
     */
    void request_redraw() const noexcept override
    {
        process_event({gui_event_type::window_redraw, layout().clipping_rectangle_on_window()});
    }

    /** Handle command.
     * If a widget does not fully handle a command it should pass the
     * command to the super class' `handle_event()`.
     */
    bool handle_event(gui_event const& event) noexcept override;

    bool handle_event_recursive(
        gui_event const& event,
        std::vector<widget_id> const& reject_list = std::vector<widget_id>{}) noexcept override;

    [[nodiscard]] virtual widget_id find_next_widget(
        widget_id current_keyboard_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept override;

    [[nodiscard]] widget_id find_first_widget(keyboard_focus_group group) const noexcept override;

    [[nodiscard]] widget_id find_last_widget(keyboard_focus_group group) const noexcept override;

    /** Is this widget the first widget in the parent container.
     */
    [[nodiscard]] bool is_first(keyboard_focus_group group) const noexcept;

    /** Is this widget the last widget in the parent container.
     */
    [[nodiscard]] bool is_last(keyboard_focus_group group) const noexcept;

    using widget_intf::scroll_to_show;
    void scroll_to_show(hi::aarectangle rectangle) noexcept override;

    void set_window(gui_window *window) noexcept override
    {
        if (parent) {
            return parent->set_window(window);
        } else {
            return;
        }
    }

    [[nodiscard]] gui_window *window() const noexcept override
    {
        if (parent) {
            return parent->window();
        } else {
            return nullptr;
        }
    }

    [[nodiscard]] hi::theme const &theme() const noexcept
    {
        hilet w = window();
        hi_assert_not_null(w);
        return w->theme;
    }

    [[nodiscard]] gfx_surface const *surface() const noexcept
    {
        if (auto w = window()) {
            return w->surface.get();
        } else {
            return nullptr;
        }
    }

    [[nodiscard]] virtual color background_color() const noexcept;

    [[nodiscard]] virtual color foreground_color() const noexcept;

    [[nodiscard]] virtual color focus_color() const noexcept;

    [[nodiscard]] virtual color accent_color() const noexcept;

    [[nodiscard]] virtual color label_color() const noexcept;

protected:
    widget_layout _layout;

    decltype(mode)::callback_token _mode_cbt;

    /** Make an overlay rectangle.
     *
     * This function tries to create a rectangle for an overlay-widget that
     * will fit on the window. It will try and keep the rectangle in the given
     * position and of the given size, but may return something smaller and shifted.
     *
     * @param requested_rectangle A rectangle in the local coordinate system.
     * @return A rectangle that fits the window's constraints in the local coordinate system.
     */
    [[nodiscard]] aarectangle make_overlay_rectangle(aarectangle requested_rectangle) const noexcept;
};

}} // namespace hi::v1
