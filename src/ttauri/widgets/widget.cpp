// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "widget.hpp"
#include "abstract_container_widget.hpp"
#include "../GUI/utils.hpp"
#include <ranges>

namespace tt {

widget::widget(gui_window &_window, std::shared_ptr<abstract_container_widget> parent) noexcept :
    enabled(true), window(_window), _parent(parent), _draw_layer(0.0f), _logical_layer(0), _semantic_layer(0)
{
    if (parent) {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        _draw_layer = parent->draw_layer() + 1.0f;
        _logical_layer = parent->logical_layer() + 1;
        _semantic_layer = parent->semantic_layer() + 1;
    }

    _enabled_callback = enabled.subscribe([this](auto...) {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        window.request_redraw(aarect{this->local_to_window() * this->clipping_rectangle()});
    });

    _preferred_size = {
        extent2{0.0f, 0.0f}, extent2{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
}

widget::~widget() {}

gui_device *widget::device() const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    auto device = window.device();
    tt_assert(device);
    return device;
}

bool widget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    need_reconstrain |= std::exchange(_request_reconstrain, false);
    return need_reconstrain;
}

void widget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    need_layout |= std::exchange(_request_relayout, false);
    if (need_layout) {
        window.request_redraw(aarect{_local_to_window * _clipping_rectangle});
    }
}

[[nodiscard]] color widget::background_color() const noexcept
{
    if (*enabled) {
        if (_hover) {
            return theme::global->fillColor(_semantic_layer + 1);
        } else {
            return theme::global->fillColor(_semantic_layer);
        }
    } else {
        return theme::global->fillColor(_semantic_layer - 1);
    }
}

[[nodiscard]] color widget::foreground_color() const noexcept
{
    if (*enabled) {
        if (_hover) {
            return theme::global->borderColor(_semantic_layer + 1);
        } else {
            return theme::global->borderColor(_semantic_layer);
        }
    } else {
        return theme::global->borderColor(_semantic_layer - 1);
    }
}

[[nodiscard]] color widget::focus_color() const noexcept
{
    if (*enabled) {
        if (_focus && window.active) {
            return theme::global->accentColor;
        } else if (_hover) {
            return theme::global->borderColor(_semantic_layer + 1);
        } else {
            return theme::global->borderColor(_semantic_layer);
        }
    } else {
        return theme::global->borderColor(_semantic_layer - 1);
    }
}

[[nodiscard]] color widget::accent_color() const noexcept
{
    if (*enabled) {
        if (window.active) {
            return theme::global->accentColor;
        } else {
            return theme::global->borderColor(_semantic_layer);
        }
    } else {
        return theme::global->borderColor(_semantic_layer - 1);
    }
}

[[nodiscard]] color widget::label_color() const noexcept
{
    if (*enabled) {
        return theme::global->labelStyle.color;
    } else {
        return theme::global->borderColor(_semantic_layer - 1);
    }
}

draw_context widget::make_draw_context(draw_context const &parent_context) const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return parent_context.make_child_context(_parent_to_local, _local_to_window, _clipping_rectangle);

}

bool widget::handle_event(command command) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    switch (command) {
        using enum tt::command;
    case gui_keyboard_enter:
        _focus = true;
        window.request_redraw(aarect{_local_to_window * _clipping_rectangle});
        return true;

    case gui_keyboard_exit:
        _focus = false;
        window.request_redraw(aarect{_local_to_window * _clipping_rectangle});
        return true;

    case gui_mouse_enter:
        _hover = true;
        window.request_redraw(aarect{_local_to_window * _clipping_rectangle});
        return true;

    case gui_mouse_exit:
        _hover = false;
        window.request_redraw(aarect{_local_to_window * _clipping_rectangle});
        return true;

    default:;
    }

    return false;
}

bool widget::handle_command_recursive(command command, std::vector<std::shared_ptr<widget>> const &reject_list) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (!std::ranges::any_of(reject_list, [this](ttlet &x) {
            return x.get() == this;
        })) {
        return handle_event(command);
    } else {
        return false;
    }
}

bool widget::handle_event(mouse_event const &event) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    return false;
}

bool widget::handle_event(keyboard_event const &event) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    return false;
}

std::shared_ptr<widget> widget::find_next_widget(
    std::shared_ptr<widget> const &current_keyboard_widget,
    keyboard_focus_group group,
    keyboard_focus_direction direction) const noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    tt_axiom(direction != keyboard_focus_direction::current);

    auto this_ = shared_from_this();
    if (current_keyboard_widget == this_) {
        // This is the current widget, this widget does not have any children, so return this.
        return std::const_pointer_cast<widget>(this_);

    } else if (!current_keyboard_widget && accepts_keyboard_focus(group)) {
        // If the current_keyboard_widget is empty, then return the first widget
        // that accepts focus.
        return std::const_pointer_cast<widget>(this_);

    } else {
        return {};
    }
}

/** Get a shared_ptr to the parent.
 */
[[nodiscard]] std::shared_ptr<abstract_container_widget const> widget::shared_parent() const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return _parent.lock();
}

/** Get a shared_ptr to the parent.
 */
[[nodiscard]] std::shared_ptr<abstract_container_widget> widget::shared_parent() noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return _parent.lock();
}

[[nodiscard]] abstract_container_widget const &widget::parent() const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    if (ttlet parent_ = shared_parent()) {
        return *parent_;
    } else {
        tt_no_default();
    }
}

[[nodiscard]] abstract_container_widget &widget::parent() noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    if (ttlet parent_ = shared_parent()) {
        return *parent_;
    } else {
        tt_no_default();
    }
}

[[nodiscard]] bool widget::is_first(keyboard_focus_group group) const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return parent().find_first_widget(group).get() == this;
}

[[nodiscard]] bool widget::is_last(keyboard_focus_group group) const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return parent().find_last_widget(group).get() == this;
}

/** Get a list of parents of a given widget.
 * The chain includes the given widget.
 */
[[nodiscard]] std::vector<std::shared_ptr<widget>> widget::parent_chain(std::shared_ptr<tt::widget> const &child_widget) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    std::vector<std::shared_ptr<widget>> chain;

    if (auto w = child_widget) {
        chain.push_back(w);
        while (w = std::static_pointer_cast<widget>(w->shared_parent())) {
            chain.push_back(w);
        }
    }

    return chain;
}

} // namespace tt
