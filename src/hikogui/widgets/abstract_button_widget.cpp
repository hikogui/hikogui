// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "abstract_button_widget.hpp"
#include "../GUI/theme.hpp"
#include "../GUI/gui_system.hpp"
#include "../loop.hpp"

namespace hi::inline v1 {

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
        _delegate_cbt = d->subscribe(*this, [&] {
            request_relayout();
        });
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

    this->pressed.post();
}

widget_constraints abstract_button_widget::set_constraints_button() const noexcept
{
    return max(_on_label_widget->set_constraints(), _off_label_widget->set_constraints(), _other_label_widget->set_constraints());
}

void abstract_button_widget::draw_button(draw_context const &context) noexcept
{
    _on_label_widget->draw(context);
    _off_label_widget->draw(context);
    _other_label_widget->draw(context);
}

void abstract_button_widget::set_layout_button(widget_layout const &context) noexcept
{
    auto state_ = state();
    _on_label_widget->visible = state_ == button_state::on;
    _off_label_widget->visible = state_ == button_state::off;
    _other_label_widget->visible = state_ == button_state::other;

    _on_label_widget->set_layout(context.transform(_label_rectangle));
    _off_label_widget->set_layout(context.transform(_label_rectangle));
    _other_label_widget->set_layout(context.transform(_label_rectangle));
}

[[nodiscard]] color abstract_button_widget::background_color() const noexcept
{
    hi_axiom(is_gui_thread());
    if (_pressed) {
        return theme().color(semantic_color::fill, semantic_layer + 2);
    } else {
        return super::background_color();
    }
}

[[nodiscard]] hitbox abstract_button_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(is_gui_thread());

    if (*visible and *enabled and layout().contains(position)) {
        return {this, position, hitbox::Type::Button};
    } else {
        return {};
    }
}

[[nodiscard]] bool abstract_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    hi_axiom(is_gui_thread());
    return *visible and *enabled and any(group & hi::keyboard_focus_group::normal);
}

void activate() noexcept;

[[nodiscard]] bool abstract_button_widget::handle_event(command command) noexcept
{
    hi_axiom(is_gui_thread());

    if (*enabled) {
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
    hi_axiom(is_gui_thread());
    auto handled = super::handle_event(event);

    if (event.cause.leftButton) {
        handled = true;
        if (*enabled) {
            if (compare_store(_pressed, static_cast<bool>(event.down.leftButton))) {
                request_redraw();
            }

            if (event.type == mouse_event::Type::ButtonUp && layout().rectangle().contains(event.position)) {
                handled |= handle_event(command::gui_activate);
            }
        }
    }
    return handled;
}

} // namespace hi::inline v1
