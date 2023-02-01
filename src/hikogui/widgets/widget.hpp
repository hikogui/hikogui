// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/widget.hpp Defines widget.
 * @ingroup widgets
 */

#pragma once

#include "widget_layout.hpp"
#include "widget_mode.hpp"
#include "../GFX/draw_context.hpp"
#include "../GUI/theme.hpp"
#include "../GUI/hitbox.hpp"
#include "../GUI/keyboard_focus_direction.hpp"
#include "../GUI/keyboard_focus_group.hpp"
#include "../GUI/gui_event.hpp"
#include "../GUI/widget_id.hpp"
#include "../layout/box_constraints.hpp"
#include "../geometry/module.hpp"
#include "../observer.hpp"
#include "../chrono.hpp"
#include "../generator.hpp"
#include "../cache.hpp"
#include "../os_settings.hpp"
#include "../tagged_id.hpp"
#include <memory>
#include <vector>
#include <string>
#include <ranges>

namespace hi { inline namespace v1 {
class gui_window;
class gfx_surface;

/** An interactive graphical object as part of the user-interface.
 *
 * Rendering is done in three distinct phases:
 *  1. Updating Constraints: `widget::update_constraints()`
 *  2. Updating Layout: `widget::set_layout()`
 *  3. Drawing: `widget::draw()`
 *
 * @ingroup widgets
 */
class widget : public std::enable_shared_from_this<widget> {
public:
    /** Pointer to the parent widget.
     * May be a nullptr only when this is the top level widget.
     */
    widget *parent = nullptr;

    /** The numeric identifier of a widget.
     *
     * @note This is a uint32_t equal to the operating system's accessibility identifier.
     */
    widget_id id = {};

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
    observer<extent2i> minimum = extent2i{};

    /** The maximum size this widget is allowed to be.
     */
    observer<extent2i> maximum = extent2i::large();

    /*! Constructor for creating sub views.
     */
    explicit widget(widget *parent) noexcept;

    virtual ~widget();
    widget(const widget&) = delete;
    widget& operator=(const widget&) = delete;
    widget(widget&&) = delete;
    widget& operator=(widget&&) = delete;

    /** Get a list of child widgets.
     */
    [[nodiscard]] virtual generator<widget const&> children(bool include_invisible) const noexcept
    {
        co_return;
    }

    [[nodiscard]] generator<widget&> children(bool include_invisible) noexcept
    {
        for (auto& child : const_cast<widget const *>(this)->children(include_invisible)) {
            co_yield const_cast<widget&>(child);
        }
    }

    /** Find the widget that is under the mouse cursor.
     * This function will recursively test with visual child widgets, when
     * widgets overlap on the screen the hitbox object with the highest elevation is returned.
     *
     * @param position The coordinate of the mouse local to the widget.
     * @return A hit_box object with the cursor-type and a reference to the widget.
     */
    [[nodiscard]] virtual hitbox hitbox_test(point2i position) const noexcept
    {
        return {};
    }

    /** Call hitbox_test from a parent widget.
     *
     * This function will transform the position from parent coordinates to local coordinates.
     *
     * @param position The coordinate of the mouse local to the parent widget.
     */
    [[nodiscard]] virtual hitbox hitbox_test_from_parent(point2i position) const noexcept
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
    [[nodiscard]] virtual hitbox hitbox_test_from_parent(point2i position, hitbox sibling_hitbox) const noexcept
    {
        return std::max(sibling_hitbox, hitbox_test(_layout.from_parent * position));
    }

    /** Check if the widget will accept keyboard focus.
     *
     */
    [[nodiscard]] virtual bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept
    {
        hi_axiom(loop::main().on_thread());
        return false;
    }

    /** Update the constraints of the widget.
     *
     * Typically the implementation of this function starts with recursively calling update_constraints()
     * on its children.
     *
     * If the container, due to a change in constraints, wants the window to resize to the minimum size
     * it should call `request_resize()`.
     *
     * @post This function will change what is returned by `widget::minimum_size()`, `widget::preferred_size()`
     *       and `widget::maximum_size()`.
     */
    virtual [[nodiscard]] box_constraints update_constraints() noexcept
    {
        _layout = {};
        return {*minimum, *minimum, *maximum};
    }

    /** Update the internal layout of the widget.
     * This function is called when the size of this widget must change, or if any of the
     * widget request a re-layout.
     *
     * This function may be used for expensive calculations, such as geometry calculations,
     * which should only be done when the data or sizes change; it should cache these calculations.
     *
     * @post This function will change what is returned by `widget::size()` and the transformation
     *       matrices.
     * @param context The layout for this child.
     */
    virtual void set_layout(widget_layout const& context) noexcept
    {
        _layout = context;
    }

    /** Get the current layout for this widget.
     */
    widget_layout const& layout() const noexcept
    {
        return _layout;
    }

    /** Draw the widget.
     *
     * This function is called by the window (optionally) on every frame.
     * It should recursively call this function on every visible child.
     * This function is only called when `updateLayout()` has returned true.
     *
     * The overriding function should call the base class's `draw()`, the place
     * where the call this function will determine the order of the vertices into
     * each buffer. This is important when needing to do the painters algorithm
     * for alpha-compositing. However the pipelines are always drawn in the same
     * order.
     *
     * @param context The context to where the widget will draw.
     */
    virtual void draw(draw_context const& context) noexcept {}

    virtual bool process_event(gui_event const& event) const noexcept
    {
        if (parent != nullptr) {
            return parent->process_event(event);
        } else {
            return true;
        }
    }

    /** Request the widget to be redrawn on the next frame.
     */
    virtual void request_redraw() const noexcept
    {
        process_event({gui_event_type::window_redraw, layout().clipping_rectangle_on_window()});
    }

    /** Handle command.
     * If a widget does not fully handle a command it should pass the
     * command to the super class' `handle_event()`.
     */
    virtual bool handle_event(gui_event const& event) noexcept;

    /** Handle command recursive.
     * Handle a command and pass it to each child.
     *
     * @param event The command to handle by this widget.
     * @param reject_list The widgets that should ignore this command
     * @return True when the command was handled by this widget or recursed child.
     */
    virtual bool
    handle_event_recursive(gui_event const& event, std::vector<widget_id> const& reject_list = std::vector<widget_id>{}) noexcept;

    /** Find the next widget that handles keyboard focus.
     * This recursively looks for the current keyboard widget, then returns the next (or previous) widget
     * that returns true from `accepts_keyboard_focus()`.
     *
     * @param current_keyboard_widget The widget that currently has focus; or nullptr to get the first widget
     *                                that accepts focus.
     * @param group The group to which the widget must belong.
     * @param direction The direction in which to walk the widget tree.
     * @return A pointer to the next widget.
     * @retval current_keyboard_widget When current_keyboard_widget was found but no next widget that accepts
                                       keyboard focus was found.
     * @retval nullptr When current_keyboard_widget is not found in this widget.
     */
    [[nodiscard]] virtual widget_id find_next_widget(
        widget_id current_keyboard_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept;

    [[nodiscard]] widget_id find_first_widget(keyboard_focus_group group) const noexcept;

    [[nodiscard]] widget_id find_last_widget(keyboard_focus_group group) const noexcept;

    /** Is this widget the first widget in the parent container.
     */
    [[nodiscard]] bool is_first(keyboard_focus_group group) const noexcept;

    /** Is this widget the last widget in the parent container.
     */
    [[nodiscard]] bool is_last(keyboard_focus_group group) const noexcept;

    /** Scroll to show the given rectangle on the window.
     * This will call parents, until all parents have scrolled
     * the rectangle to be shown on the window.
     *
     * @param rectangle The rectangle in window coordinates.
     */
    virtual void scroll_to_show(hi::aarectanglei rectangle) noexcept;

    /** Scroll to show the important part of the widget.
     */
    void scroll_to_show() noexcept
    {
        scroll_to_show(layout().rectangle());
    }

    /** Get a list of parents of a given widget.
     * The chain includes the given widget.
     */
    [[nodiscard]] std::vector<widget_id> parent_chain() const noexcept;

    [[nodiscard]] virtual gui_window *window() const noexcept
    {
        if (parent) {
            return parent->window();
        } else {
            return nullptr;
        }
    }

    [[nodiscard]] virtual hi::theme const& theme() const noexcept
    {
        hi_assert_not_null(parent);
        return parent->theme();
    }

    [[nodiscard]] virtual gfx_surface const *surface() const noexcept
    {
        if (parent) {
            return parent->surface();
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
    [[nodiscard]] aarectanglei make_overlay_rectangle(aarectanglei requested_rectangle) const noexcept;
};

inline widget *get_if(widget *start, widget_id id, bool include_invisible) noexcept
{
    hi_assert_not_null(start);

    if (start->id == id) {
        return start;
    }
    for (auto& child : start->children(include_invisible)) {
        if (hilet r = get_if(&child, id, include_invisible); r != nullptr) {
            return r;
        }
    }
    return nullptr;
}

inline widget& get(widget& start, widget_id id, bool include_invisible)
{
    if (auto r = get_if(std::addressof(start), id, include_invisible); r != nullptr) {
        return *r;
    }
    throw not_found_error("get widget by id");
}

}} // namespace hi::v1
