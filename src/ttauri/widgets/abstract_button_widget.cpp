// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "abstract_button_widget.hpp"
#include "../GUI/theme.hpp"
#include "../GUI/gui_system.hpp"

namespace tt {

abstract_button_widget::abstract_button_widget(
    gui_window &window,
    widget *parent,
    weak_or_unique_ptr<delegate_type> delegate) noexcept :
    super(window, parent), _delegate(std::move(delegate))
{
    _on_label_widget = std::make_unique<label_widget>(window, this, on_label, label_alignment);
    _off_label_widget = std::make_unique<label_widget>(window, this, off_label, label_alignment);
    _other_label_widget = std::make_unique<label_widget>(window, this, other_label, label_alignment);
    if (auto d = _delegate.lock()) {
        d->subscribe(*this, _relayout_callback);
        d->init(*this);
    }
}

abstract_button_widget::~abstract_button_widget()
{
    if (auto delegate = _delegate.lock()) {
        delegate->deinit(*this);
    }
}

void abstract_button_widget::activate() noexcept
{
    if (auto delegate = _delegate.lock()) {
        delegate->activate(*this);
    }

    window.gui.run_from_event_queue([this]() {
        this->_notifier();
    });
}

/** Unsubscribe a callback.
 */
void abstract_button_widget::unsubscribe(callback_ptr_type &callback_ptr) noexcept
{
    tt_axiom(is_gui_thread());
    return _notifier.unsubscribe(callback_ptr);
}

[[nodiscard]] bool
abstract_button_widget::constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::constrain(display_time_point, need_reconstrain)) {
        _minimum_size = _on_label_widget->minimum_size();
        _preferred_size = _on_label_widget->preferred_size();
        _maximum_size = _on_label_widget->maximum_size();

        _minimum_size = max(_minimum_size, _off_label_widget->minimum_size());
        _preferred_size = max(_preferred_size, _off_label_widget->preferred_size());
        _maximum_size = max(_maximum_size, _off_label_widget->maximum_size());

        _minimum_size = max(_minimum_size, _other_label_widget->minimum_size());
        _preferred_size = max(_preferred_size, _other_label_widget->preferred_size());
        _maximum_size = max(_maximum_size, _other_label_widget->maximum_size());

        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void
abstract_button_widget::layout(utc_nanoseconds displayTimePoint, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _request_layout.exchange(false);
    if (need_layout) {
        auto state_ = state();
        _on_label_widget->visible = state_ == button_state::on;
        _off_label_widget->visible = state_ == button_state::off;
        _other_label_widget->visible = state_ == button_state::other;

        _on_label_widget->set_layout_parameters_from_parent(_label_rectangle);
        _off_label_widget->set_layout_parameters_from_parent(_label_rectangle);
        _other_label_widget->set_layout_parameters_from_parent(_label_rectangle);
    }
    widget::layout(displayTimePoint, need_layout);
}

[[nodiscard]] color abstract_button_widget::background_color() const noexcept
{
    tt_axiom(is_gui_thread());
    if (_pressed) {
        return theme().color(theme_color::fill, semantic_layer + 2);
    } else {
        return super::background_color();
    }
}

[[nodiscard]] hitbox abstract_button_widget::hitbox_test(point2 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (_visible_rectangle.contains(position)) {
        return hitbox{this, draw_layer, enabled ? hitbox::Type::Button : hitbox::Type::Default};
    } else {
        return hitbox{};
    }
}

[[nodiscard]] bool abstract_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());
    return is_normal(group) and enabled;
}

void activate() noexcept;

[[nodiscard]] bool abstract_button_widget::handle_event(command command) noexcept
{
    tt_axiom(is_gui_thread());

    if (enabled) {
        switch (command) {
        case command::gui_activate: activate(); return true;
        case command::gui_enter:
            activate();
            window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::forward);
            return true;
        default:;
        }
    }

    return super::handle_event(command);
}

[[nodiscard]] bool abstract_button_widget::handle_event(mouse_event const &event) noexcept
{
    tt_axiom(is_gui_thread());
    auto handled = super::handle_event(event);

    if (event.cause.leftButton) {
        handled = true;
        if (enabled) {
            if (compare_then_assign(_pressed, static_cast<bool>(event.down.leftButton))) {
                request_redraw();
            }

            if (event.type == mouse_event::Type::ButtonUp && rectangle().contains(event.position)) {
                handled |= handle_event(command::gui_activate);
            }
        }
    }
    return handled;
}

}