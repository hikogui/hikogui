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

widget_constraints const &abstract_button_widget::set_constraints_button() noexcept
{
    tt_axiom(is_gui_thread());

    _on_label_widget->set_constraints();
    _off_label_widget->set_constraints();
    _other_label_widget->set_constraints();

    _constraints.min = _on_label_widget->constraints().min;
    _constraints.pref = _on_label_widget->constraints().pref;
    _constraints.max = _on_label_widget->constraints().max;

    _constraints.min = max(_constraints.min, _off_label_widget->constraints().min);
    _constraints.pref = max(_constraints.pref, _off_label_widget->constraints().pref);
    _constraints.max = max(_constraints.max, _off_label_widget->constraints().max);

    _constraints.min = max(_constraints.min, _other_label_widget->constraints().min);
    _constraints.pref = max(_constraints.pref, _other_label_widget->constraints().pref);
    _constraints.max = max(_constraints.max, _other_label_widget->constraints().max);

    tt_axiom(_constraints.min <= _constraints.pref && _constraints.pref <= _constraints.max);
    return _constraints;
}

void abstract_button_widget::draw_button(draw_context const &context) noexcept
{
    if (_on_label_widget->visible) {
        _on_label_widget->draw(context);
    }
    if (_off_label_widget->visible) {
        _off_label_widget->draw(context);
    }
    if (_other_label_widget->visible) {
        _other_label_widget->draw(context);
    }
}

void abstract_button_widget::set_layout_button(widget_layout const &context) noexcept
{
    tt_axiom(is_gui_thread());

    auto state_ = state();
    _on_label_widget->visible = state_ == button_state::on;
    _off_label_widget->visible = state_ == button_state::off;
    _other_label_widget->visible = state_ == button_state::other;

    _on_label_widget->set_layout(_label_rectangle * context);
    _off_label_widget->set_layout(_label_rectangle * context);
    _other_label_widget->set_layout(_label_rectangle * context);
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

[[nodiscard]] hitbox abstract_button_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (layout().hit_rectangle.contains(position)) {
        return hitbox{this, position, enabled ? hitbox::Type::Button : hitbox::Type::Default};
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

            if (event.type == mouse_event::Type::ButtonUp && layout().rectangle().contains(event.position)) {
                handled |= handle_event(command::gui_activate);
            }
        }
    }
    return handled;
}

} // namespace tt