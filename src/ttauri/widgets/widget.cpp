// Copyright 2019 Pokitec
// All rights reserved.

#include "widget.hpp"
#include "abstract_container_widget.hpp"
#include "../GUI/utils.hpp"
#include <ranges>

namespace tt {

widget::widget(gui_window &_window, std::shared_ptr<abstract_container_widget> _parent) noexcept :
    enabled(true),
    window(_window),
    parent(_parent),
    _draw_layer(0.0f),
    _logical_layer(0),
    _semantic_layer(0)
{
    if (_parent) {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        _draw_layer = _parent->draw_layer() + 1.0f;
        _logical_layer = _parent->logical_layer() + 1;
        _semantic_layer = _parent->semantic_layer() + 1;
    }

    _enabled_callback = enabled.subscribe([this](auto...) {
        window.request_redraw(window_clipping_rectangle());
    });

    _preferred_size = {
        f32x4{0.0f, 0.0f},
        f32x4{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}
    };
}

widget::~widget()
{
}

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
        window.request_redraw(window_clipping_rectangle());

        // Used by draw().
        _to_window_transform = mat::T(_window_rectangle.x(), _window_rectangle.y(), _draw_layer);

        // Used by handle_mouse_event()
        _from_window_transform = ~_to_window_transform;
    }
}

draw_context widget::make_draw_context(draw_context context) const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    context.clipping_rectangle = _window_clipping_rectangle;
    context.transform = _to_window_transform;

    // The default fill and border colors.
    context.color = theme::global->borderColor(_semantic_layer);
    context.fill_color = theme::global->fillColor(_semantic_layer);

    if (*enabled) {
        if (_focus && window.active) {
            context.color = theme::global->accentColor;
        } else if (_hover) {
            context.color = theme::global->borderColor(_semantic_layer + 1);
        }

        if (_hover) {
            context.fill_color = theme::global->fillColor(_semantic_layer + 1);
        }

    } else {
        // Disabled, only the outline is shown.
        context.color = theme::global->borderColor(_semantic_layer - 1);
        context.fill_color = theme::global->fillColor(_semantic_layer - 1);
    }

    return context;
}

bool widget::handle_command(command command) noexcept
{
    return false;
}

bool widget::handle_command_recursive(command command, std::vector<std::shared_ptr<widget>> const &reject_list) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (!std::ranges::any_of(reject_list, [this](ttlet &x) { return x.get() == this; })) {
        return handle_command(command);
    } else {
        return false;
    }
}

bool widget::handle_mouse_event(MouseEvent const &event) noexcept {
    ttlet lock = std::scoped_lock(gui_system_mutex);
    auto handled = false;

    if (event.type == MouseEvent::Type::Entered) {
        handled = true;
        _hover = true;
        window.request_redraw(window_clipping_rectangle());

    } else if (event.type == MouseEvent::Type::Exited) {
        handled = true;
        _hover = false;
        window.request_redraw(window_clipping_rectangle());
    }
    return handled;
}

bool widget::handle_keyboard_event(KeyboardEvent const &event) noexcept {
    ttlet lock = std::scoped_lock(gui_system_mutex);
    auto handled = false;

    switch (event.type) {
    case KeyboardEvent::Type::Entered:
        handled = true;
        _focus = true;
        window.request_redraw(window_clipping_rectangle());
        break;

    case KeyboardEvent::Type::Exited:
        handled = true;
        _focus = false;
        window.request_redraw(window_clipping_rectangle());
        break;

    default:;
    }

    return handled;
}

std::shared_ptr<widget>
widget::find_next_widget(std::shared_ptr<widget> const &current_keyboard_widget, keyboard_focus_group group, keyboard_focus_direction direction) const noexcept
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

/** Get a list of parents of a given widget.
 * The chain includes the given widget.
 */
std::vector<std::shared_ptr<widget>> widget::parent_chain(std::shared_ptr<tt::widget> const &child_widget) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    std::vector<std::shared_ptr<widget>> chain;

    if (auto w = child_widget) {
        chain.push_back(w);
        while (w = std::static_pointer_cast<widget>(w->parent.lock())) {
            chain.push_back(w);
        }
    }

    return chain;
}

}
