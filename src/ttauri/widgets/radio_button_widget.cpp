// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "radio_button_widget.hpp"

namespace tt::inline v1 {

widget_constraints const &radio_button_widget::set_constraints() noexcept
{
    _layout = {};

    // Make room for button and margin.
    _button_size = {theme().size, theme().size};
    ttlet extra_size = extent2{theme().margin + _button_size.width(), 0.0f};
    _constraints = max(set_constraints_button() + extra_size, _button_size);
    _constraints.margins = theme().margin;
    return _constraints;
}

void radio_button_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        _button_rectangle = align(layout.rectangle(), _button_size, alignment::top_left());

        _label_rectangle = aarectangle{_button_rectangle.right() + theme().margin, 0.0f, layout.width(), layout.height()};

        _pip_rectangle = align(_button_rectangle, extent2{theme().icon_size, theme().icon_size}, alignment::middle_center());
    }
    set_layout_button(layout);
}

void radio_button_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        draw_radio_button(context);
        draw_radio_pip(context);
        draw_button(context);
    }
}

void radio_button_widget::draw_radio_button(draw_context const &context) noexcept
{
    context.draw_circle(
        layout(),
        circle{_button_rectangle} * 1.02f,
        background_color(),
        focus_color(),
        theme().border_width,
        border_side::inside);
}

void radio_button_widget::draw_radio_pip(draw_context const &context) noexcept
{
    _animated_value.update(state() == button_state::on ? 1.0f : 0.0f, context.display_time_point);
    if (_animated_value.is_animating()) {
        request_redraw();
    }

    // draw pip
    auto float_value = _animated_value.current_value();
    if (float_value > 0.0) {
        context.draw_circle(layout(), circle{_pip_rectangle} * 1.02f * float_value, accent_color());
    }
}

} // namespace tt::inline v1
