// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "hitbox.hpp"
#include "widget_layout.hpp"
#include "widget_id.hpp"
#include "widget_state.hpp"
#include "keyboard_focus_group.hpp"
#include "../layout/layout.hpp"
#include "../GFX/GFX.hpp"
#include "../telemetry/telemetry.hpp"
#include "../theme/theme.hpp"
#include "../macros.hpp"
#include <coroutine>

hi_export_module(hikogui.GUI : widget_intf);

hi_export namespace hi {
inline namespace v1 {
class gui_window;

class widget_intf {
public:
    /** The numeric identifier of a widget.
     *
     * @note This is a uint32_t equal to the operating system's accessibility identifier.
     */
    widget_id id = {};

    /** The style of this widget.
     * 
     * You can assign a style-string to this style variable to change
     * the style's id, class and individual style-attributes.
     * @see hi::parse_style().
     */
    hi::style style = {};

    gui_window *window = nullptr;

    /** Notifier which is called after an action is completed by a widget.
     */
    notifier<void()> notifier;

    /** The current state of the widget.
     */
    observer<widget_state> state;

    virtual ~widget_intf()
    {
        release_widget_id(id);
    }

    widget_intf() noexcept : id(make_widget_id())
    {
        _style_cbt = style.subscribe([&](style_modify_mask mask, bool path_has_changed) {
            if (path_has_changed) {
                // When the path has changed of a style, the style of the children
                // may change as well. Therefor we update the parent-path of the
                // children to trigger a re-evaluation of the style.
                ++global_counter<"widget:style:path">;
                for (auto &child : children()) {
                    child.style.set_parent_path(style.path());
                }
            }

            if (to_bool(mask & style_modify_mask::layout)) {
                // The layout has changed which means its size may have changed
                // which would require a re-layout and re-constrain of the widget.
                ++global_counter<"widget:style:reconstrain">;
                process_event({gui_event_type::window_reconstrain});

            } else if (to_bool(mask & style_modify_mask::redraw)) {
                // The color attributes, or border magnitude has changed which
                // should only change the widget visually and not the layout.
                ++global_counter<"widget:style:redraw">;
                request_redraw();
            }
        });

        // This lambda allows the state to be set once before it will trigger
        // notifications.
        _state_cbt = state.subscribe([&](widget_state new_state) {
            style.set_pseudo_class(new_state.pseudo_class());

            static std::optional<widget_state> old_state = std::nullopt;

            if (old_state) {
                if (need_reconstrain(*old_state, *state)) {
                    ++global_counter<"widget:state:reconstrain">;
                    process_event({gui_event_type::window_reconstrain});

                } else if (need_relayout(*old_state, *state)) {
                    ++global_counter<"widget:state:relayout">;
                    process_event({gui_event_type::window_relayout});

                } else if (need_redraw(*old_state, *state)) {
                    ++global_counter<"widget:state:redraw">;
                    request_redraw();
                }
            }
            old_state = *state;
        });
    }

    /** Pointer to the parent widget.
     * 
     * May be a nullptr only when this is the top level widget, or when
     * the widget is removed from its parent.
     */
    [[nodiscard]] widget_intf *parent() const noexcept
    {
        return _parent;
    }

    /** Set the parent widget.
     * 
     * @param new_parent A pointer to an existing parent, or nullptr if the
     *                   widget is removed from the parent.
     */
    virtual void set_parent(widget_intf *new_parent) noexcept;

    /** Subscribe a callback to be called when an action is completed by the widget.
     */
    template<forward_of<void()> Func>
    [[nodiscard]] callback<void()> subscribe(Func&& func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return notifier.subscribe(std::forward<Func>(func), flags);
    }

    /** Await until an action is completed by the widget.
     */
    [[nodiscard]] auto operator co_await() const noexcept
    {
        return notifier.operator co_await();
    }

    [[nodiscard]] size_t layer() const noexcept
    {
        return state->layer();
    }

    void set_layer(size_t new_layer) noexcept
    {
        state->set_layer(new_layer);
    }

    [[nodiscard]] widget_mode mode() const noexcept
    {
        return state->mode();
    }

    void set_mode(widget_mode new_mode) noexcept
    {
        state->set_mode(new_mode);
    }

    [[nodiscard]] widget_value value() const noexcept
    {
        return state->value();
    }

    void set_value(widget_value new_value) noexcept
    {
        state->set_value(new_value);
    }

    [[nodiscard]] widget_phase phase() const noexcept
    {
        return state->phase();
    }

    void set_pressed(bool pressed) noexcept
    {
        state->set_pressed(pressed);
    }

    void set_hover(bool hover) noexcept
    {
        state->set_hover(hover);
    }

    void set_active(bool active) noexcept
    {
        state->set_active(active);
    }

    [[nodiscard]] bool focus() const noexcept
    {
        return state->focus();
    }

    void set_focus(bool new_focus) noexcept
    {
        state->set_focus(new_focus);
    }

    /** Get a list of child widgets.
     */
    [[nodiscard]] virtual generator<widget_intf&> children(bool include_invisible = true) noexcept = 0;

    /** Get a list of child widgets.
     */
    [[nodiscard]] virtual generator<widget_intf const&> children(bool include_invisible = true) const noexcept final
    {
        for (auto& child : const_cast<widget_intf *>(this)->children(include_invisible)) {
            co_yield child;
        }
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
    [[nodiscard]] virtual box_constraints update_constraints() noexcept = 0;

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
    virtual void set_layout(widget_layout const& context) noexcept = 0;

    /** Get the current layout for this widget.
     */
    [[nodiscard]] widget_layout const& layout() const noexcept
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
    virtual void draw(draw_context const& context) noexcept = 0;

    /** Find the widget that is under the mouse cursor.
     * This function will recursively test with visual child widgets, when
     * widgets overlap on the screen the hitbox object with the highest elevation is returned.
     *
     * @param position The coordinate of the mouse local to the widget.
     * @return A hit_box object with the cursor-type and a reference to the widget.
     */
    [[nodiscard]] virtual hitbox hitbox_test(point2 position) const noexcept = 0;

    /** Check if the widget will accept keyboard focus.
     *
     */
    [[nodiscard]] virtual bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept = 0;

    /** Request the widget to be redrawn on the next frame.
     */
    virtual void request_redraw() const noexcept = 0;

    /** Send a event to the window.
     */
    virtual bool process_event(gui_event const& event) const noexcept = 0;

    /** Handle command.
     * If a widget does not fully handle a command it should pass the
     * command to the super class' `handle_event()`.
     */
    virtual bool handle_event(gui_event const& event) noexcept = 0;

    /** Handle command recursive.
     * Handle a command and pass it to each child.
     *
     * @param event The command to handle by this widget.
     * @param reject_list The widgets that should ignore this command
     * @return True when the command was handled by this widget or recursed child.
     */
    virtual bool handle_event_recursive(
        gui_event const& event,
        std::vector<widget_id> const& reject_list = std::vector<widget_id>{}) noexcept = 0;

    /** Find the next widget that handles keyboard focus.
     * This recursively looks for the current keyboard widget, then returns the
     * next (or previous) widget that returns true from
     * `accepts_keyboard_focus()`.
     *
     * @param current_keyboard_widget The widget that currently has focus; or
     *        nullptr to get the first widget that accepts focus.
     * @param group The group to which the widget must belong.
     * @param direction The direction in which to walk the widget tree.
     * @return A pointer to the next widget.
     * @retval current_keyboard_widget When current_keyboard_widget was found
     *         but no next widget that accepts keyboard focus was found.
     * @retval nullptr When current_keyboard_widget is not found in this widget.
     */
    [[nodiscard]] virtual widget_id find_next_widget(
        widget_id current_keyboard_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept = 0;

    /** Get a list of parents of a given widget.
     * The chain includes the given widget.
     */
    [[nodiscard]] std::vector<widget_id> parent_chain() const noexcept
    {
        std::vector<widget_id> chain;

        if (auto w = this) {
            chain.push_back(w->id);
            while ((w = w->parent())) {
                chain.push_back(w->id);
            }
        }

        return chain;
    }

    /** Scroll to show the given rectangle on the window.
     * This will call parents, until all parents have scrolled
     * the rectangle to be shown on the window.
     *
     * @param rectangle The rectangle in window coordinates.
     */
    virtual void scroll_to_show(hi::aarectangle rectangle) noexcept = 0;

    /** Scroll to show the important part of the widget.
     */
    void scroll_to_show() noexcept
    {
        scroll_to_show(layout().rectangle());
    }

protected:
    callback<void(widget_state)> _state_cbt;
    hi::style::callback_type _style_cbt;

    widget_layout _layout;

private:
    widget_intf *_parent = nullptr;
};

inline widget_intf *get_if(widget_intf *start, widget_id id, bool include_invisible) noexcept
{
    hi_assert_not_null(start);

    if (start->id == id) {
        return start;
    }
    for (auto& child : start->children(include_invisible)) {
        if (auto const r = get_if(&child, id, include_invisible); r != nullptr) {
            return r;
        }
    }
    return nullptr;
}

inline widget_intf& get(widget_intf& start, widget_id id, bool include_invisible)
{
    if (auto r = get_if(std::addressof(start), id, include_invisible); r != nullptr) {
        return *r;
    }
    throw not_found_error("get widget by id");
}

template<std::invocable<widget_intf&> Func>
inline void apply(widget_intf& start, Func &&func, bool include_invisible = true)
{
    auto todo = std::vector<widget_intf *>{&start};

    while (not todo.empty()) {
        auto *tmp = todo.back();
        todo.pop_back();

        func(*tmp);

        for (auto &child : tmp->children(include_invisible)) {
            todo.push_back(&child);
        }
    }
}

inline void apply_window_data(widget_intf& start, gui_window *new_window, pixel_density const& new_density, style::attributes_from_theme_type const& new_attributes_from_theme)
{
    apply(start, [&](widget_intf& w) {
        w.window = new_window;
        w.style.set_pixel_density(new_density);
        w.style.set_attributes_from_theme(new_attributes_from_theme);
    });
}

void widget_intf::set_parent(widget_intf *new_parent) noexcept
{
    _parent = new_parent;
    apply_window_data(
        *this,
        new_parent ? new_parent->window : nullptr,
        new_parent ? new_parent->style.pixel_density() : pixel_density{},
        new_parent ? new_parent->style.attributes_from_theme() : style::attributes_from_theme_type{});

    // The path will automatically propagate to the child widgets.
    style.set_parent_path(new_parent ? new_parent->style.path() : style_path{});
}


} // namespace v1
} // namespace hi
