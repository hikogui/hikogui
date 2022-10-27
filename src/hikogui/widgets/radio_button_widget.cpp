// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "radio_button_widget.hpp"

namespace hi::inline v1 {

widget_constraints const &radio_button_widget::set_constraints() noexcept
{
    _layout = {};

    // Make room for button and margin.
    _button_size = {theme().size, theme().size};
    hilet extra_size = extent2{theme().margin + _button_size.width(), 0.0f};
    _constraints = max(set_constraints_button() + extra_size, _button_size);
    _constraints.margins = theme().margin;
    _constraints.baseline = widget_baseline{0.5f, alignment->vertical(), theme().cap_height, theme().size};
    return _constraints;
}

void radio_button_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        auto alignment_ = layout.left_to_right() ? *alignment : mirror(*alignment);

        if (alignment_ == horizontal_alignment::left or alignment_ == horizontal_alignment::right) {
            _button_rectangle = round(align(layout.rectangle(), _button_size, alignment_));
        } else {
            hi_not_implemented();
        }

        hilet label_width = layout.width() - (_button_rectangle.width() + theme().margin);
        if (alignment_ == horizontal_alignment::left) {
            hilet label_left = _button_rectangle.right() + theme().margin;
            _label_rectangle = aarectangle{label_left, 0.0f, label_width, layout.height()};

        } else if (alignment_ == horizontal_alignment::right) {
            _label_rectangle = aarectangle{0.0f, 0.0f, label_width, layout.height()};
        } else {
            hi_not_implemented();
        }

        _pip_rectangle = align(_button_rectangle, extent2{theme().icon_size, theme().icon_size}, alignment::middle_center());
    }
    set_layout_button(layout);
}

void radio_button_widget::draw(draw_context const &context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
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

} // namespace hi::inline v1
