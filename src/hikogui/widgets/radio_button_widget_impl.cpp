// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "radio_button_widget.hpp"

namespace hi::inline v1 {

[[nodiscard]] box_constraints radio_button_widget::update_constraints() noexcept
{
    _label_constraints = super::update_constraints();

    // Make room for button and margin.
    _button_size = {theme().size(), theme().size()};
    hilet extra_size = extent2i{theme().margin<int>() + _button_size.width(), 0};

    auto constraints = max(_label_constraints + extra_size, _button_size);
    constraints.margins = theme().margin();
    constraints.alignment = *alignment;
    return constraints;
}

void radio_button_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        auto alignment_ = os_settings::left_to_right() ? *alignment : mirror(*alignment);

        if (alignment_ == horizontal_alignment::left or alignment_ == horizontal_alignment::right) {
            _button_rectangle = align(context.rectangle(), _button_size, alignment_);
        } else {
            hi_not_implemented();
        }

        hilet label_width = context.width() - (_button_rectangle.width() + theme().margin<int>());
        if (alignment_ == horizontal_alignment::left) {
            hilet label_left = _button_rectangle.right() + theme().margin<int>();
            hilet label_rectangle = aarectanglei{label_left, 0, label_width, context.height()};
            _on_label_shape = _off_label_shape = _other_label_shape =
                box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};

        } else if (alignment_ == horizontal_alignment::right) {
            hilet label_rectangle = aarectanglei{0, 0, label_width, context.height()};
            _on_label_shape = _off_label_shape = _other_label_shape =
                box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};

        } else {
            hi_not_implemented();
        }

        _button_circle = circle{narrow_cast<aarectangle>(_button_rectangle)};

        _pip_circle = align(
            narrow_cast<aarectangle>(_button_rectangle),
            circle{theme().size() * 0.5f - 3.0f},
            alignment::middle_center());
    }
    super::set_layout(context);
}

void radio_button_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        draw_radio_button(context);
        draw_radio_pip(context);
        draw_button(context);
    }
}

void radio_button_widget::draw_radio_button(draw_context const& context) noexcept
{
    context.draw_circle(
        layout(),
        _button_circle * 1.02f,
        background_color(),
        focus_color(),
        theme().border_width(),
        border_side::inside);
}

void radio_button_widget::draw_radio_pip(draw_context const& context) noexcept
{
    _animated_value.update(state() == button_state::on ? 1.0f : 0.0f, context.display_time_point);
    if (_animated_value.is_animating()) {
        request_redraw();
    }

    // draw pip
    auto float_value = _animated_value.current_value();
    if (float_value > 0.0) {
        context.draw_circle(layout(), _pip_circle * 1.02f * float_value, accent_color());
    }
}

} // namespace hi::inline v1
