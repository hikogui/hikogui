// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "abstract_button_widget.hpp"
#include "../GUI/theme.hpp"
#include "../loop.hpp"

namespace hi::inline v1 {

abstract_button_widget::abstract_button_widget(widget *parent, std::shared_ptr<delegate_type> delegate) noexcept :
    super(parent), delegate(std::move(delegate))
{
    hi_assert_not_null(this->delegate);

    _on_label_widget = std::make_unique<label_widget>(this, on_label, alignment, text_style);
    _off_label_widget = std::make_unique<label_widget>(this, off_label, alignment, text_style);
    _other_label_widget = std::make_unique<label_widget>(this, other_label, alignment, text_style);
    _delegate_cbt = this->delegate->subscribe([&] {
        ++global_counter<"abstract_button_widget:delegate:relayout">;
        process_event({gui_event_type::window_relayout});
    });
    this->delegate->init(*this);
}

abstract_button_widget::~abstract_button_widget()
{
    hi_assert_not_null(delegate);
    delegate->deinit(*this);
}

[[nodiscard]] box_constraints abstract_button_widget::update_constraints() noexcept
{
    _layout = {};
    _on_label_constraints = _on_label_widget->update_constraints();
    _off_label_constraints = _off_label_widget->update_constraints();
    _other_label_constraints = _other_label_widget->update_constraints();
    return max(_on_label_constraints, _off_label_constraints, _other_label_constraints);
}

void abstract_button_widget::set_layout(widget_layout const& context) noexcept
{
    auto state_ = state();
    _on_label_widget->mode = state_ == button_state::on ? widget_mode::display : widget_mode::invisible;
    _off_label_widget->mode = state_ == button_state::off ? widget_mode::display : widget_mode::invisible;
    _other_label_widget->mode = state_ == button_state::other ? widget_mode::display : widget_mode::invisible;

    _on_label_widget->set_layout(context.transform(_on_label_shape));
    _off_label_widget->set_layout(context.transform(_off_label_shape));
    _other_label_widget->set_layout(context.transform(_other_label_shape));
}

void abstract_button_widget::activate() noexcept
{
    hi_assert_not_null(delegate);
    delegate->activate(*this);

    this->pressed();
}

void abstract_button_widget::draw_button(draw_context const& context) noexcept
{
    _on_label_widget->draw(context);
    _off_label_widget->draw(context);
    _other_label_widget->draw(context);
}

[[nodiscard]] color abstract_button_widget::background_color() const noexcept
{
    hi_axiom(loop::main().on_thread());
    if (_pressed) {
        return theme().color(semantic_color::fill, semantic_layer + 2);
    } else {
        return super::background_color();
    }
}

[[nodiscard]] hitbox abstract_button_widget::hitbox_test(point2i position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    if (*mode >= widget_mode::partial and layout().contains(position)) {
        return {id, _layout.elevation, hitbox_type::button};
    } else {
        return {};
    }
}

[[nodiscard]] bool abstract_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    hi_axiom(loop::main().on_thread());
    return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::normal);
}

void activate() noexcept;

bool abstract_button_widget::handle_event(gui_event const& event) noexcept
{
    hi_axiom(loop::main().on_thread());

    switch (event.type()) {
    case gui_event_type::gui_activate:
        if (*mode >= widget_mode::partial) {
            activate();
            return true;
        }
        break;

    case gui_event_type::mouse_down:
        if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
            _pressed = true;
            request_redraw();
            return true;
        }
        break;

    case gui_event_type::mouse_up:
        if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
            _pressed = false;

            if (layout().rectangle().contains(event.mouse().position)) {
                handle_event(gui_event_type::gui_activate);
            }
            request_redraw();
            return true;
        }
        break;

    default:;
    }

    return super::handle_event(event);
}

} // namespace hi::inline v1
