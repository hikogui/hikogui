// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget_id.hpp"
#include "widget_mode.hpp"
#include "widget_state.hpp"
#include "widget_layout.hpp"
#include "hitbox.hpp"
#include "keyboard_focus_group.hpp"
#include "gui_event.hpp"
#include "widget_draw_context.hpp"
#include "theme_value.hpp"
#include "../geometry/module.hpp"
#include "../layout/box_constraints.hpp"
#include "../observer.hpp"
#include "../generator.hpp"
#include "../loop.hpp"

namespace hi { inline namespace v1 {
class gui_window;
class gfx_surface;

class widget {
public:
    using callback_token = notifier<void()>::callback_token;
    using awaiter_type = notifier<void()>::awaiter_type;

    /** The numeric identifier of a widget.
     *
     * @note This is a uint32_t equal to the operating system's accessibility identifier.
     */
    widget_id id = {};

    /** Pointer to the parent widget.
     * May be a nullptr only when this is the top level widget.
     */
    widget *parent = nullptr;

    /** The window this widget is on.
     */
    gui_window *window = nullptr;

    /** The surface this widget is drawn on.
     */
    gfx_surface *surface = nullptr;

    /** The widget mode.
     * The current visibility and interactivity of a widget.
     */
    observer<widget_mode> mode = widget_mode::enabled;

    /** Mouse cursor is hovering over the widget.
     */
    observer<bool> hover = false;

    /** The widget is being clicked by the mouse.
     */
    observer<bool> clicked = false;

    /** The widget has keyboard focus.
     */
    observer<bool> focus = false;

    /** The state of the widget.
     */
    observer<widget_state> state = widget_state::off;

    /** The scale by which to size the widget and its components.
     *
     * This value is set by the window when it's dpi changes.
     */
    float dpi_scale = 1.0f;

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
    size_t semantic_layer = 0_uz;

    /** The minimum size this widget is allowed to be.
     */
    observer<extent2i> minimum = extent2i{};

    /** The maximum size this widget is allowed to be.
     */
    observer<extent2i> maximum = extent2i::large();

    widget_layout layout;

    virtual ~widget() {}
    widget(widget *parent) noexcept : parent(parent), id(narrow_cast<uint32_t>(++global_counter<"widget::id">))
    {
        hi_axiom(loop::main().on_thread());

        if (parent) {
            semantic_layer = parent->semantic_layer + 1;
        }

        _mode_cbt = mode.subscribe([&](auto...) {
            ++global_counter<"widget:mode:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });
    }

    widget(widget const&) = delete;
    widget(widget&&) = delete;
    widget& operator=(widget&&) = delete;
    widget& operator=(widget const&) = delete;

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

    /** Get an index to select a color from a color theme.
     *
     * The color theme index is based on the state of the widget:
     *  - [1:0] disabled=0,enabled=1,hover=2,clicked=3
     *  - [2:2] keyboard-focus
     *  - [3:3] widget-state != off
     *  - [5:4] semantic-layer modulo 4.
     */
    [[nodiscard]] uint8_t color_theme_index() const noexcept
    {
        auto r = [&] {
            if (*mode == widget_mode::disabled) {
                return uint8_t{0};
            } else if (*clicked) {
                return uint8_t{3};
            } else if (*hover) {
                return uint8_t{2};
            } else {
                return uint8_t{1};
            }
        }();

        if (*focus) {
            r |= 4;
        }

        if (*state != widget_state::off) {
            r |= 8;
        }

        r |= (semantic_layer % 4) << 4;
        hi_axiom(r <= 31);
        return r;
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
        return hitbox_test(layout.from_parent * position);
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
        return std::max(sibling_hitbox, hitbox_test(layout.from_parent * position));
    }

    /** Check if the widget will accept keyboard focus.
     *
     */
    [[nodiscard]] virtual bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept
    {
        return false;
    }

    /** Reset the layout.
     *
     * @param dpi_scale The dpi-scale to use from this point forward.
     */
    void reset_layout(gui_window *new_window, gfx_surface *new_surface, float new_dpi_scale) noexcept
    {
        layout = {};
        window = new_window;
        surface = new_surface;
        dpi_scale = new_dpi_scale;
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
    [[nodiscard]] virtual box_constraints update_constraints() noexcept
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
    virtual void set_layout(widget_layout const& context) noexcept
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
    virtual void draw(widget_draw_context const& context) noexcept {};

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
        process_event({gui_event_type::window_redraw, layout.clipping_rectangle_on_window()});
    }

    /** Handle command.
     * If a widget does not fully handle a command it should pass the
     * command to the super class' `handle_event()`.
     */
    virtual bool handle_event(gui_event const& event) noexcept
    {
        hi_axiom(loop::main().on_thread());

        switch (event.type()) {
            using enum hi::gui_event_type;
        case keyboard_enter:
            focus = true;
            scroll_to_show(layout.rectangle());
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
    virtual bool
    handle_event_recursive(gui_event const& event, std::vector<widget_id> const& reject_list = std::vector<widget_id>{}) noexcept
    {
        hi_axiom(loop::main().on_thread());

        auto handled = false;

        for (auto& child : children(false)) {
            handled |= child.handle_event_recursive(event, reject_list);
        }

        if (not std::ranges::any_of(reject_list, [&](hilet& x) {
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
    [[nodiscard]] virtual widget_id find_next_widget(
        widget_id current_keyboard_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept
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

    [[nodiscard]] widget_id find_first_widget(keyboard_focus_group group) const noexcept
    {
        hi_axiom(loop::main().on_thread());

        for (auto& child : children(false)) {
            if (child.accepts_keyboard_focus(group)) {
                return child.id;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] widget_id find_last_widget(keyboard_focus_group group) const noexcept
    {
        hi_axiom(loop::main().on_thread());

        auto found = widget_id{};
        for (auto& child : children(false)) {
            if (child.accepts_keyboard_focus(group)) {
                found = child.id;
            }
        }

        return found;
    }

    [[nodiscard]] bool is_first(keyboard_focus_group group) const noexcept
    {
        hi_axiom(loop::main().on_thread());
        return parent->find_first_widget(group) == id;
    }

    [[nodiscard]] bool is_last(keyboard_focus_group group) const noexcept
    {
        hi_axiom(loop::main().on_thread());
        return parent->find_last_widget(group) == id;
    }

    /** Scroll to show the given rectangle on the window.
     * This will call parents, until all parents have scrolled
     * the rectangle to be shown on the window.
     *
     * @param rectangle The rectangle in window coordinates.
     */
    virtual void scroll_to_show(hi::aarectanglei rectangle) noexcept
    {
        hi_axiom(loop::main().on_thread());

        if (parent) {
            parent->scroll_to_show(layout.to_parent * rectangle);
        }
    }

    /** Get a list of parents of a given widget.
     * The chain includes the given widget.
     */
    [[nodiscard]] std::vector<widget_id> parent_chain() const noexcept
    {
        hi_axiom(loop::main().on_thread());

        std::vector<widget_id> chain;

        if (auto w = this) {
            chain.push_back(w->id);
            while (to_bool(w = w->parent)) {
                chain.push_back(w->id);
            }
        }

        return chain;
    }

    /** Check if this widget is a tab-button.
     *
     * Used by the toolbar to determine if it will draw a focus line underneath
     * the toolbar.
     */
    [[nodiscard]] virtual bool is_tab_button() const noexcept
    {
        return false;
    }

    template<forward_of<void()> Callback>
    [[nodiscard]] callback_token subscribe(Callback&& callback, callback_flags flags = callback_flags::synchronous) const noexcept
    {
        return _state_changed.subscribe(std::forward<Callback>(callback), flags);
    }

    [[nodiscard]] awaiter_type operator co_await() const noexcept
    {
        return _state_changed.operator co_await();
    }

protected:
    /** Notifier which is called whenever the use modifies the state.
     */
    mutable notifier<void()> _state_changed;

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
        // We can't use theme<> here to get the margin, because of circular dependency.
        hilet margin = narrow_cast<int>(std::ceil(5.0f * dpi_scale));
        hilet window_bounds = aarectanglei{layout.window_size} - margin;
        hilet response_window_rectangle = fit(window_bounds, requested_window_rectangle);
        return layout.from_window * response_window_rectangle;
    }

private:
    decltype(mode)::callback_token _mode_cbt;
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

template<std::invocable<widget&> F>
inline void apply(widget& start, bool include_invisible, F const& f)
{
    for (auto& child : start.children(include_invisible)) {
        apply(child, include_invisible, f);
    }
    f(start);
}

template<std::invocable<widget&> F>
inline void apply(widget& start, F const& f)
{
    return apply(start, true, f);
}

template<fixed_string Tag, std::floating_point T>
struct theme<Tag, T> {
    [[nodiscard]] T operator()(widget const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        hilet& value = detail::global_theme_value<Tag, float>;
        return wide_cast<T>(value.get() * widget->dpi_scale);
    }
};

template<fixed_string Tag, std::integral T>
struct theme<Tag, T> {
    [[nodiscard]] T operator()(widget const *widget) const noexcept
    {
        return narrow_cast<T>(std::ceil(theme<Tag, float>{}(widget)));
    }
};

template<fixed_string Tag>
struct theme<Tag, extent2i> {
    [[nodiscard]] extent2i operator()(widget const *widget) const noexcept
    {
        hilet tmp = theme<Tag, int>{}(widget);
        return extent2i{tmp, tmp};
    }
};

template<fixed_string Tag>
struct theme<Tag, marginsi> {
    [[nodiscard]] marginsi operator()(widget const *widget) const noexcept
    {
        hilet tmp = theme<Tag, int>{}(widget);
        return marginsi{tmp};
    }
};

template<fixed_string Tag>
struct theme<Tag, corner_radii> {
    [[nodiscard]] corner_radii operator()(widget const *widget) const noexcept
    {
        hilet tmp = theme<Tag, float>{}(widget);
        return corner_radii{tmp};
    }
};

template<fixed_string Tag>
struct theme<Tag, hi::color> {
    [[nodiscard]] hi::color operator()(widget const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        hilet& value = detail::global_theme_value<Tag, hi::color>;
        return value.get(widget->color_theme_index());
    }
};

template<fixed_string Tag>
struct theme<Tag, text_theme> {
    [[nodiscard]] text_theme operator()(widget const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        hilet& value = detail::global_theme_value<Tag, hi::text_theme>;
        return value.get();
    }
};

}} // namespace hi::v1
