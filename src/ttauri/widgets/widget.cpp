// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "widget.hpp"
#include "abstract_container_widget.hpp"
#include "../GUI/utils.hpp"
#include <ranges>

namespace tt {

widget::widget(
    gui_window &_window,
    std::shared_ptr<abstract_container_widget> parent,
    std::shared_ptr<widget_delegate> delegate) noexcept :
    window(_window), _delegate(std::move(delegate)), _parent(std::move(parent)), _draw_layer(0.0f), _logical_layer(0), _semantic_layer(0)
{
    if (auto p = _parent.lock()) {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        _draw_layer = p->draw_layer() + 1.0f;
        _logical_layer = p->logical_layer() + 1;
        _semantic_layer = p->semantic_layer() + 1;
    }

    _delegate_callback = _delegate->subscribe([this](widget_update_level level) {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        
        switch (level) {
        case widget_update_level::redraw:
            this->request_redraw();
            break;
        case widget_update_level::layout:
            _request_relayout = true;
            break;
        case widget_update_level::constrain:
            _request_reconstrain = true;
            break;
        default:
            tt_no_default();
        }
    });

    _minimum_size = extent2::nan();
    _preferred_size = extent2::nan();
    _maximum_size = extent2::nan();
}

widget::~widget() {}

void widget::init() noexcept
{
    return _delegate->init(*this);
}

void widget::deinit() noexcept
{
    return _delegate->deinit(*this);
}

    [[nodiscard]] bool widget::enabled() const noexcept
    {
        return _delegate->enabled(*this);
    }

void widget::set_enabled(observable<bool> rhs) noexcept
{
    return _delegate->set_enabled(*this, std::move(rhs));
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
        request_redraw();
    }
}

[[nodiscard]] color widget::background_color() const noexcept
{
    if (enabled()) {
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
    if (enabled()) {
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
    if (enabled()) {
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
    if (enabled()) {
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
    if (enabled()) {
        return theme::global->labelStyle.color;
    } else {
        return theme::global->borderColor(_semantic_layer - 1);
    }
}

bool widget::handle_event(command command) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

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

void widget::scroll_to_show(tt::rectangle rectangle) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (auto parent = _parent.lock()) {
        parent->scroll_to_show(_local_to_parent * rectangle);
    }
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
