// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "radio_button_widget.hpp"

namespace tt {

widget_constraints const &radio_button_widget::set_constraints() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};
    set_constraints_button();

    // Make room for button and margin.
    _button_size = {theme().size, theme().size};
    ttlet extra_size = extent2{theme().margin + _button_size.width(), 0.0f};
    _constraints.min += extra_size;
    _constraints.pref += extra_size;
    _constraints.max += extra_size;

    _constraints.min = max(_constraints.min, _button_size);
    _constraints.pref = max(_constraints.min, _button_size);
    _constraints.max = max(_constraints.min, _button_size);

    tt_axiom(_constraints.min <= _constraints.pref && _constraints.pref <= _constraints.max);
    return _constraints;
}

void radio_button_widget::set_layout(widget_layout const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        if (_layout.store(context) >= layout_update::transform) {
            _button_rectangle = align(layout().rectangle(), _button_size, alignment::top_left);

            _label_rectangle = aarectangle{_button_rectangle.right() + theme().margin, 0.0f, layout().width(), layout().height()};

            _pip_rectangle = align(_button_rectangle, extent2{theme().icon_size, theme().icon_size}, alignment::middle_center);
        }
        set_layout_button(context);
    }
}

void radio_button_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, layout())) {
        draw_radio_button(context);
        draw_radio_pip(context);
        draw_button(context);
    }
}

void radio_button_widget::draw_radio_button(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    context.draw_box_with_border_inside(
        layout(), _button_rectangle, background_color(), focus_color(), corner_shapes{_button_rectangle.height() * 0.5f});
}

void radio_button_widget::draw_radio_pip(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    _animated_value.update(state() == button_state::on ? 1.0f : 0.0f, context.display_time_point);
    if (_animated_value.is_animating()) {
        request_redraw();
    }

    // draw pip
    auto float_value = _animated_value.current_value();
    if (float_value > 0.0) {
        ttlet scaled_pip_rectangle = _pip_rectangle * float_value;
        context.draw_box(layout(), scaled_pip_rectangle, accent_color(), corner_shapes{scaled_pip_rectangle.height() * 0.5f});
    }
}

} // namespace tt
