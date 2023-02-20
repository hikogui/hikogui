// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/widget.hpp Defines widget.
 * @ingroup widgets
 */

#pragma once

#include "../GFX/draw_context.hpp"
#include "../GUI/hitbox.hpp"
#include "../GUI/keyboard_focus_direction.hpp"
#include "../GUI/keyboard_focus_group.hpp"
#include "../GUI/gui_event.hpp"
#include "../GUI/widget_intf.hpp"
#include "../GUI/theme_value.hpp"
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
template<fixed_string Tag>
class widget : public widget_intf {
public:
    using super = widget_intf;

    /** The (custom)-name of the widget.
     *
     * This is of the format:
     *  - "<name>.<widget-type>."
     *  - "<widget-type>."
     */
    constexpr static auto tag = Tag;

    /** The minimum size this widget is allowed to be.
     */
    observer<extent2i> minimum = extent2i{};

    /** The maximum size this widget is allowed to be.
     */
    observer<extent2i> maximum = extent2i::large();

    /*! Constructor for creating sub views.
     */
    explicit widget(widget_intf *parent) noexcept : super(parent), , logical_layer(0), semantic_layer(0)
    {
        hi_axiom(loop::main().on_thread());

        if (parent) {
            logical_layer = parent->logical_layer + 1;
            semantic_layer = parent->semantic_layer + 1;
        }

        _mode_cbt = mode.subscribe([&](auto...) {
            ++global_counter<"widget:mode:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });
    }

    virtual ~widget(){};

    /** Get a list of child widgets.
     */
    [[nodiscard]] generator<widget_intf const&> children(bool include_invisible) const noexcept override
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
    [[nodiscard]] hitbox hitbox_test(point2i position) const noexcept override
    {
        return {};
    }

    /** Call hitbox_test from a parent widget.
     *
     * This function will transform the position from parent coordinates to local coordinates.
     *
     * @param position The coordinate of the mouse local to the parent widget.
     */
    [[nodiscard]] hitbox hitbox_test_from_parent(point2i position) const noexcept override
    {
        return hitbox_test(layout.from_parent * position);
    }

    /** Call hitbox_test from a parent widget.
     *
     * This function will transform the position from parent coordinates to local coordinates.
     *
     * @param position The coordinate of the mouse local to the parent widget.
     * @param sibling_hitbox The hitbox of a sibling to combine with the hitbox of this widget.
     */
    [[nodiscard]] hitbox hitbox_test_from_parent(point2i position, hitbox sibling_hitbox) const noexcept override
    {
        return std::max(sibling_hitbox, hitbox_test(layout.from_parent * position));
    }

    /** Check if the widget will accept keyboard focus.
     *
     */
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        return false;
    }

    /** Reset the layout before constraining.
     */
    void reset_layout(float new_dpi_scale) noexcept override
    {
        dpi_scale = new_dpi_scale;
        layout = {};
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
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
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
    void set_layout(widget_layout const& context) noexcept override
    {
        layout = context;
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
    void draw(draw_context const& context) noexcept override {}

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
        process_event({gui_event_type::window_redraw, layout.clipping_rectangle_on_window()});
    }

    /** Handle command.
     * If a widget does not fully handle a command it should pass the
     * command to the super class' `handle_event()`.
     */
    bool handle_event(gui_event const& event) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        switch (event.type()) {
            using enum hi::gui_event_type;
        case keyboard_enter:
            focus = true;
            scroll_to_show();
            ++global_counter<"widget:keyboard_enter:redraw">;
            request_redraw();
            return true;

        case keyboard_exit:
            focus = false;
            ++global_counter<"widget:keyboard_exit:redraw">;
            request_redraw();
            return true;

        case mouse_enter:
            hover = true;
            ++global_counter<"widget:mouse_enter:redraw">;
            request_redraw();
            return true;

        case mouse_exit:
            hover = false;
            ++global_counter<"widget:mouse_exit:redraw">;
            request_redraw();
            return true;

        case gui_widget_next:
            process_event(
                gui_event::window_set_keyboard_target(id, keyboard_focus_group::normal, keyboard_focus_direction::forward));
            return true;

        case gui_widget_prev:
            process_event(
                gui_event::window_set_keyboard_target(id, keyboard_focus_group::normal, keyboard_focus_direction::backward));
            return true;

        case gui_activate_next:
            process_event(gui_activate);
            return process_event(gui_widget_next);

        case gui_event_type::gui_toolbar_next:
            if (*mode >= widget_mode::partial and accepts_keyboard_focus(keyboard_focus_group::toolbar) and
                not is_last(keyboard_focus_group::toolbar)) {
                process_event(
                    gui_event::window_set_keyboard_target(id, keyboard_focus_group::toolbar, keyboard_focus_direction::forward));
                return true;
            }
            break;

        case gui_event_type::gui_toolbar_prev:
            if (*mode >= widget_mode::partial and accepts_keyboard_focus(keyboard_focus_group::toolbar) and
                not is_first(keyboard_focus_group::toolbar)) {
                process_event(
                    gui_event::window_set_keyboard_target(id, keyboard_focus_group::toolbar, keyboard_focus_direction::backward));
                return true;
            }
            break;

        default:;
        }

        return false;
    }

    /** Handle command recursive.
     * Handle a command and pass it to each child.
     *
     * @param event The command to handle by this widget.
     * @param reject_list The widgets that should ignore this command
     * @return True when the command was handled by this widget or recursed child.
     */
    bool handle_event_recursive(
        gui_event const& event,
        std::vector<widget_id> const& reject_list = std::vector<widget_id>{}) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        auto handled = false;

        for (auto& child : children(false)) {
            handled |= child.handle_event_recursive(event, reject_list);
        }

        if (!std::ranges::any_of(reject_list, [&](hilet& x) {
                return x == id;
            })) {
            handled |= handle_event(event);
        }

        return handled;
    }
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
    [[nodiscard]] widget_id find_next_widget(
        widget_id current_keyboard_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        auto found = false;

        if (not current_keyboard_widget and accepts_keyboard_focus(group)) {
            // If there was no current_keyboard_widget, then return this if it accepts focus.
            return id;

        } else if (current_keyboard_widget == id) {
            found = true;
        }

        auto children_ = std::vector<widget const *>{};
        for (auto& child : children(false)) {
            children_.push_back(std::addressof(child));
        }

        if (direction == keyboard_focus_direction::backward) {
            std::reverse(begin(children_), end(children_));
        }

        for (auto *child : children_) {
            hi_axiom_not_null(child);

            if (found) {
                // Find the first focus accepting widget.
                if (auto tmp = child->find_next_widget({}, group, direction)) {
                    return tmp;
                }

            } else {
                auto tmp = child->find_next_widget(current_keyboard_widget, group, direction);
                if (tmp == current_keyboard_widget) {
                    // The current widget was found, but no next widget available in the child.
                    // Try the first widget that does accept keyboard focus.
                    found = true;

                } else if (tmp != nullptr) {
                    // Return the next widget that was found in the child-widget.
                    return tmp;
                }
            }
        }

        if (found) {
            // Either:
            // 1. current_keyboard_widget was nullptr; this widget, nor its child widgets accept focus.
            // 2. current_keyboard_wigget was this; none of the child widgets accept focus.
            // 3. current_keyboard_widget is a child; none of the following widgets accept focus.
            return current_keyboard_widget;
        }

        return std::nullopt;
    }
    /** Scroll to show the given rectangle on the window.
     * This will call parents, until all parents have scrolled
     * the rectangle to be shown on the window.
     *
     * @param rectangle The rectangle in window coordinates.
     */
    void scroll_to_show(hi::aarectanglei rectangle) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (parent) {
            parent->scroll_to_show(layout.to_parent * rectangle);
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

    [[nodiscard]] gfx_surface const *surface() const noexcept override
    {
        if (parent) {
            return parent->surface();
        } else {
            return nullptr;
        }
    }

protected:
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
    [[nodiscard]] aarectanglei make_overlay_rectangle(aarectanglei requested_rectangle) const noexcept
    {
        hi_axiom(loop::main().on_thread());

        // Move the request_rectangle to window coordinates.
        hilet requested_window_rectangle = translate2i{layout.clipping_rectangle_on_window()} * requested_rectangle;
        hilet window_bounds = aarectanglei{layout.window_size} - theme().margin<int>();
        hilet response_window_rectangle = fit(window_bounds, requested_window_rectangle);
        return layout.from_window * response_window_rectangle;
    }
};

}} // namespace hi::v1
