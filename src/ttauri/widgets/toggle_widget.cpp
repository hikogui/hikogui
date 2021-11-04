// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toggle_widget.hpp"

namespace tt {

toggle_widget::toggle_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept :
    super(window, parent, std::move(delegate))
{
    label_alignment = alignment::top_left;
}

toggle_widget::toggle_widget(gui_window &window, widget *parent, std::unique_ptr<delegate_type> delegate) noexcept :
    toggle_widget(window, parent, weak_or_unique_ptr<delegate_type>{std::move(delegate)})
{
}

widget_constraints const &toggle_widget::set_constraints() noexcept
{
    _layout = {};

    // Make room for button and margin.
    _button_size = {theme().size * 2.0f, theme().size};
    ttlet extra_size = extent2{theme().margin + _button_size.width(), 0.0f};
    _constraints = max(set_constraints_button() + extra_size, _button_size);
    _constraints.margin = theme().margin;
    return _constraints;
}

void toggle_widget::set_layout(widget_layout const &context) noexcept
{
    if (visible) {
        if (_layout.store(context) >= layout_update::transform) {
            _button_rectangle = align(layout().rectangle(), _button_size, alignment::top_left);

            _label_rectangle = aarectangle{_button_rectangle.right() + theme().margin, 0.0f, layout().width(), layout().height()};

            ttlet button_square =
                aarectangle{get<0>(_button_rectangle), extent2{_button_rectangle.height(), _button_rectangle.height()}};

            _pip_rectangle = align(button_square, extent2{theme().icon_size, theme().icon_size}, alignment::middle_center);

            ttlet pip_to_button_margin_x2 = _button_rectangle.height() - _pip_rectangle.height();
            _pip_move_range = _button_rectangle.width() - _pip_rectangle.width() - pip_to_button_margin_x2;
        }
        set_layout_button(context);
    }
}

void toggle_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        draw_toggle_button(context);
        draw_toggle_pip(context);
        draw_button(context);
    }
}

void toggle_widget::draw_toggle_button(draw_context const &context) noexcept
{
    context.draw_box(
        layout(),
        _button_rectangle,
        background_color(),
        focus_color(),
        theme().border_width,
        border_side::inside,
        corner_shapes{_button_rectangle.height() * 0.5f});
}

void toggle_widget::draw_toggle_pip(draw_context const &context) noexcept
{
    _animated_value.update(state() == button_state::on ? 1.0f : 0.0f, context.display_time_point);
    if (_animated_value.is_animating()) {
        request_redraw();
    }

    ttlet positioned_pip_rectangle = translate3{_pip_move_range * _animated_value.current_value(), 0.0f, 0.1f} * _pip_rectangle;

    ttlet forground_color_ = state() == button_state::on ? accent_color() : foreground_color();
    context.draw_box(
        layout(), positioned_pip_rectangle, forground_color_, corner_shapes{positioned_pip_rectangle.height() * 0.5f});
}

} // namespace tt
