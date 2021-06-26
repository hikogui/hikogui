// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "widget.hpp"
#include <ranges>

namespace tt {

widget::widget(gui_window &_window, widget *parent) noexcept :
    window(_window), parent(parent), _draw_layer(0.0f), _logical_layer(0), _semantic_layer(0)
{
    tt_axiom(is_gui_thread());

    if (parent) {
        _draw_layer = parent->draw_layer() + 1.0f;
        _logical_layer = parent->logical_layer() + 1;
        _semantic_layer = parent->semantic_layer() + 1;
    }

    _redraw_callback = std::make_shared<std::function<void()>>([this] {
        request_redraw();
    });

    _relayout_callback = std::make_shared<std::function<void()>>([this] {
        _request_relayout = true;
    });
    _reconstrain_callback = std::make_shared<std::function<void()>>([this] {
        _request_reconstrain = true;
    });

    enabled.subscribe(_redraw_callback);
    visible.subscribe(_redraw_callback);

    _minimum_size = extent2::nan();
    _preferred_size = extent2::nan();
    _maximum_size = extent2::nan();
}

widget::~widget() {
    // The window must remove references such as mouse and keyboard targets to
    // this widget when it is removed.
    window.widget_is_destructing(this);
}

void widget::init() noexcept {}

void widget::deinit() noexcept {}

[[nodiscard]] color widget::background_color() const noexcept
{
    if (enabled) {
        if (_hover) {
            return theme::global(theme_color::fill, _semantic_layer + 1);
        } else {
            return theme::global(theme_color::fill, _semantic_layer);
        }
    } else {
        return theme::global(theme_color::fill, _semantic_layer - 1);
    }
}

[[nodiscard]] color widget::foreground_color() const noexcept
{
    if (enabled) {
        if (_hover) {
            return theme::global(theme_color::border, _semantic_layer + 1);
        } else {
            return theme::global(theme_color::border, _semantic_layer);
        }
    } else {
        return theme::global(theme_color::border, _semantic_layer - 1);
    }
}

[[nodiscard]] color widget::focus_color() const noexcept
{
    if (enabled) {
        if (_focus && window.active) {
            return theme::global(theme_color::accent);
        } else if (_hover) {
            return theme::global(theme_color::border, _semantic_layer + 1);
        } else {
            return theme::global(theme_color::border, _semantic_layer);
        }
    } else {
        return theme::global(theme_color::border, _semantic_layer - 1);
    }
}

[[nodiscard]] color widget::accent_color() const noexcept
{
    if (enabled) {
        if (window.active) {
            return theme::global(theme_color::accent);
        } else {
            return theme::global(theme_color::border, _semantic_layer);
        }
    } else {
        return theme::global(theme_color::border, _semantic_layer - 1);
    }
}

[[nodiscard]] color widget::label_color() const noexcept
{
    if (enabled) {
        return theme::global(theme_text_style::label).color;
    } else {
        return theme::global(theme_color::border, _semantic_layer - 1);
    }
}

[[nodiscard]] bool widget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    need_reconstrain |= _request_reconstrain.exchange(false);

    for (auto &&child : _children) {
        tt_axiom(child);
        tt_axiom(child->parent == this);
        need_reconstrain |= child->update_constraints(display_time_point, need_reconstrain);
    }

    return need_reconstrain;
}

void widget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _request_relayout.exchange(false);
    for (auto &&child : _children) {
        tt_axiom(child);
        tt_axiom(child->parent == this);
        if (child->visible) {
            child->update_layout(display_time_point, need_layout);
        }
    }

    if (need_layout) {
        request_redraw();
    }
}

void widget::draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    for (auto &child : _children) {
        tt_axiom(child);
        tt_axiom(child->parent == this);

        if (child->visible) {
            auto child_context =
                context.make_child_context(child->parent_to_local(), child->local_to_window(), child->clipping_rectangle());
            child->draw(child_context, display_time_point);
        }
    }
}

[[nodiscard]] hitbox widget::hitbox_test(point2 position) const noexcept
{
    tt_axiom(is_gui_thread());

    auto r = hitbox{};
    for (ttlet &child : _children) {
        tt_axiom(child);
        tt_axiom(child->parent == this);
        if (child->visible) {
            r = std::max(r, child->hitbox_test(point2{child->parent_to_local() * position}));
        }
    }
    return r;
}

bool widget::handle_event(command command) noexcept
{
    tt_axiom(is_gui_thread());

    switch (command) {
        using enum tt::command;
    case gui_keyboard_enter:
        _focus = true;
        // When scrolling, include the margin, so that the widget is clear from the edge of the
        // scroll view's aperture.
        scroll_to_show(expand(rectangle(), _margin));
        request_redraw();
        return true;

    case gui_keyboard_exit:
        _focus = false;
        request_redraw();
        return true;

    case gui_mouse_enter:
        _hover = true;
        request_redraw();
        return true;

    case gui_mouse_exit:
        _hover = false;
        request_redraw();
        return true;

    default:;
    }

    return false;
}

bool widget::handle_command_recursive(command command, std::vector<widget const *> const &reject_list) noexcept
{
    tt_axiom(is_gui_thread());

    auto handled = false;
    for (auto &child : _children) {
        tt_axiom(child);
        tt_axiom(child->parent == this);
        handled |= child->handle_command_recursive(command, reject_list);
    }

    if (!std::ranges::any_of(reject_list, [this](ttlet &x) {
            return x == this;
        })) {
        handled |= handle_event(command);
    }

    return handled;
}

bool widget::handle_event(mouse_event const &event) noexcept
{
    tt_axiom(is_gui_thread());
    return false;
}

bool widget::handle_event(keyboard_event const &event) noexcept
{
    tt_axiom(is_gui_thread());
    return false;
}

widget const *widget::find_next_widget(
    widget const *current_keyboard_widget,
    keyboard_focus_group group,
    keyboard_focus_direction direction) const noexcept
{
    tt_axiom(is_gui_thread());

    auto found = false;

    if (!current_keyboard_widget && accepts_keyboard_focus(group)) {
        // If there was no current_keyboard_widget, then return this if it accepts focus.
        return this;

    } else if (current_keyboard_widget == this) {
        // If current_keyboard_widget is this, then we need to find the first child widget that accepts focus.
        found = true;
    }

    ssize_t first = direction == keyboard_focus_direction::forward ? 0 : ssize(_children) - 1;
    ssize_t last = direction == keyboard_focus_direction::forward ? ssize(_children) : -1;
    ssize_t step = direction == keyboard_focus_direction::forward ? 1 : -1;
    for (ssize_t i = first; i != last; i += step) {
        auto &&child = _children[i];
        tt_axiom(child);

        if (found) {
            // Find the first focus accepting widget.
            if (auto tmp = child->find_next_widget({}, group, direction)) {
                return tmp;
            }

        } else {
            auto tmp = child->find_next_widget(current_keyboard_widget, group, direction);
            if (tmp == current_keyboard_widget) {
                // The current widget was found, but no next widget available in the child.
                found = true;

            } else if (tmp) {
                return tmp;
            }
        }
    }

    if (found) {
        // Either:
        // 1. current_keyboard_widget was {} and this widget, nor its child widgets accept focus.
        // 2. current_keyboard_wigget was this and non of the child widgets accept focus.
        // 3. current_keyboard_widget is a child, and non of the following widgets accept focus.
        return current_keyboard_widget;
    }

    return nullptr;
}

[[nodiscard]] widget const *widget::find_first_widget(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());

    for (ttlet &child : _children) {
        if (child->accepts_keyboard_focus(group)) {
            return &(*child);
        }
    }
    return nullptr;
}

[[nodiscard]] widget const *widget::find_last_widget(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());

    for (ttlet &child : std::views::reverse(_children)) {
        if (child->accepts_keyboard_focus(group)) {
            return &(*child);
        }
    }
    return nullptr;
}

[[nodiscard]] bool widget::is_first(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());
    return parent->find_first_widget(group) == this;
}

[[nodiscard]] bool widget::is_last(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());
    return parent->find_last_widget(group) == this;
}

void widget::scroll_to_show(tt::rectangle rectangle) noexcept
{
    tt_axiom(is_gui_thread());

    if (parent) {
        parent->scroll_to_show(_local_to_parent * rectangle);
    }
}

/** Get a list of parents of a given widget.
 * The chain includes the given widget.
 */
[[nodiscard]] std::vector<widget const *> widget::parent_chain() const noexcept
{
    tt_axiom(is_gui_thread());

    std::vector<widget const *> chain;

    if (auto w = this) {
        chain.push_back(w);
        while (static_cast<bool>(w = w->parent)) {
            chain.push_back(w);
        }
    }

    return chain;
}

} // namespace tt
