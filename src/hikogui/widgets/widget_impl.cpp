// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../ranges.hpp"
#include "../counters.hpp"
#include <ranges>

namespace hi::inline v1 {

widget::widget(widget *parent) noexcept :
    parent(parent), id(narrow_cast<uint32_t>(++global_counter<"widget::id">)), logical_layer(0), semantic_layer(0)
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

widget::~widget() {}

[[nodiscard]] color widget::background_color() const noexcept
{
    if (*mode >= widget_mode::partial) {
        if (*hover) {
            return theme().color(semantic_color::fill, semantic_layer + 1);
        } else {
            return theme().color(semantic_color::fill, semantic_layer);
        }
    } else {
        return theme().color(semantic_color::fill, semantic_layer - 1);
    }
}

[[nodiscard]] color widget::foreground_color() const noexcept
{
    if (*mode >= widget_mode::partial) {
        if (*hover) {
            return theme().color(semantic_color::border, semantic_layer + 1);
        } else {
            return theme().color(semantic_color::border, semantic_layer);
        }
    } else {
        return theme().color(semantic_color::border, semantic_layer - 1);
    }
}

[[nodiscard]] color widget::focus_color() const noexcept
{
    if (*mode >= widget_mode::partial) {
        if (*focus) {
            return theme().color(semantic_color::accent);
        } else if (*hover) {
            return theme().color(semantic_color::border, semantic_layer + 1);
        } else {
            return theme().color(semantic_color::border, semantic_layer);
        }
    } else {
        return theme().color(semantic_color::border, semantic_layer - 1);
    }
}

[[nodiscard]] color widget::accent_color() const noexcept
{
    if (*mode >= widget_mode::partial) {
        return theme().color(semantic_color::accent);
    } else {
        return theme().color(semantic_color::border, semantic_layer - 1);
    }
}

[[nodiscard]] color widget::label_color() const noexcept
{
    if (*mode >= widget_mode::partial) {
        return theme().text_style(semantic_text_style::label)->color;
    } else {
        return theme().color(semantic_color::border, semantic_layer - 1);
    }
}

bool widget::handle_event(gui_event const& event) noexcept
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
        process_event(gui_event::window_set_keyboard_target(id, keyboard_focus_group::normal, keyboard_focus_direction::forward));
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

bool widget::handle_event_recursive(gui_event const& event, std::vector<widget_id> const& reject_list) noexcept
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

widget_id widget::find_next_widget(
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

[[nodiscard]] widget_id widget::find_first_widget(keyboard_focus_group group) const noexcept
{
    hi_axiom(loop::main().on_thread());

    for (auto& child : children(false)) {
        if (child.accepts_keyboard_focus(group)) {
            return child.id;
        }
    }
    return std::nullopt;
}

[[nodiscard]] widget_id widget::find_last_widget(keyboard_focus_group group) const noexcept
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

[[nodiscard]] bool widget::is_first(keyboard_focus_group group) const noexcept
{
    hi_axiom(loop::main().on_thread());
    return parent->find_first_widget(group) == id;
}

[[nodiscard]] bool widget::is_last(keyboard_focus_group group) const noexcept
{
    hi_axiom(loop::main().on_thread());
    return parent->find_last_widget(group) == id;
}

void widget::scroll_to_show(hi::aarectanglei rectangle) noexcept
{
    hi_axiom(loop::main().on_thread());

    if (parent) {
        parent->scroll_to_show(_layout.to_parent * rectangle);
    }
}

/** Get a list of parents of a given widget.
 * The chain includes the given widget.
 */
[[nodiscard]] std::vector<widget_id> widget::parent_chain() const noexcept
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

[[nodiscard]] aarectanglei widget::make_overlay_rectangle(aarectanglei requested_rectangle) const noexcept
{
    hi_axiom(loop::main().on_thread());

    // Move the request_rectangle to window coordinates.
    hilet requested_window_rectangle = translate2i{layout().clipping_rectangle_on_window()} * requested_rectangle;
    hilet window_bounds = aarectanglei{layout().window_size} - theme().margin<int>();
    hilet response_window_rectangle = fit(window_bounds, requested_window_rectangle);
    return layout().from_window * response_window_rectangle;
}

} // namespace hi::inline v1
