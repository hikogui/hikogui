// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "widget.hpp"
#include "../GUI/utils.hpp"
#include <ranges>

namespace tt {

widget::widget(
    gui_window &_window,
    std::shared_ptr<widget> parent,
    std::shared_ptr<widget_delegate> delegate) noexcept :
    window(_window),
    _delegate(std::move(delegate)),
    _parent(std::move(parent)),
    _draw_layer(0.0f),
    _logical_layer(0),
    _semantic_layer(0)
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
        case widget_update_level::redraw: this->request_redraw(); break;
        case widget_update_level::layout: _request_relayout = true; break;
        case widget_update_level::constrain: _request_reconstrain = true; break;
        default: tt_no_default();
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
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return _delegate->enabled(*this);
}

void widget::set_enabled(observable<bool> rhs) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return _delegate->set_enabled(*this, std::move(rhs));
}

[[nodiscard]] bool widget::visible() const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return _delegate->visible(*this);
}

void widget::set_visible(observable<bool> rhs) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return _delegate->set_visible(*this, std::move(rhs));
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

[[nodiscard]] bool widget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    auto has_constrainted = std::exchange(_request_reconstrain, false);

    for (auto &&child : _children) {
        tt_axiom(child);
        tt_axiom(&child->parent() == this);
        has_constrainted |= child->update_constraints(display_time_point, need_reconstrain);
    }

    return has_constrainted;
}

void widget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    need_layout |= std::exchange(_request_relayout, false);
    for (auto &&child : _children) {
        tt_axiom(child);
        tt_axiom(&child->parent() == this);
        child->update_layout(display_time_point, need_layout);
    }

    if (need_layout) {
        request_redraw();
    }
}

void widget::draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    for (auto &child : _children) {
        tt_axiom(child);
        tt_axiom(&child->parent() == this);

        auto child_context =
            context.make_child_context(child->parent_to_local(), child->local_to_window(), child->clipping_rectangle());
        child->draw(child_context, display_time_point);
    }
}

[[nodiscard]] hit_box widget::hitbox_test(point2 position) const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    auto r = hit_box{};
    for (ttlet &child : _children) {
        tt_axiom(child);
        tt_axiom(&child->parent() == this);
        r = std::max(r, child->hitbox_test(point2{child->parent_to_local() * position}));
    }
    return r;
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

    auto handled = false;
    for (auto &child : _children) {
        tt_axiom(child);
        tt_axiom(&child->parent() == this);
        handled |= child->handle_command_recursive(command, reject_list);
    }

    if (!std::ranges::any_of(reject_list, [this](ttlet &x) {
            return x.get() == this;
        })) {
        handled |= handle_event(command);
    }

    return handled;
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

    auto found = false;

    if (!current_keyboard_widget && accepts_keyboard_focus(group)) {
        // If there was no current_keyboard_widget, then return this if it accepts focus.
        return std::const_pointer_cast<widget>(shared_from_this());

    } else if (current_keyboard_widget == shared_from_this()) {
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

    return {};
}

[[nodiscard]] std::shared_ptr<widget const> widget::find_first_widget(keyboard_focus_group group) const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    for (ttlet child : _children) {
        if (child->accepts_keyboard_focus(group)) {
            return child;
        }
    }
    return {};
}

[[nodiscard]] std::shared_ptr<widget const> widget::find_last_widget(keyboard_focus_group group) const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    for (ttlet child : std::views::reverse(_children)) {
        if (child->accepts_keyboard_focus(group)) {
            return child;
        }
    }
    return {};
}

/** Get a shared_ptr to the parent.
 */
[[nodiscard]] std::shared_ptr<widget const> widget::shared_parent() const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return _parent.lock();
}

/** Get a shared_ptr to the parent.
 */
[[nodiscard]] std::shared_ptr<widget> widget::shared_parent() noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    return _parent.lock();
}

[[nodiscard]] widget const &widget::parent() const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    if (ttlet parent_ = shared_parent()) {
        return *parent_;
    } else {
        tt_no_default();
    }
}

[[nodiscard]] widget &widget::parent() noexcept
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
