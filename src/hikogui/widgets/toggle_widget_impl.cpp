// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toggle_widget.hpp"

namespace hi::inline v1 {

[[nodiscard]] box_constraints toggle_widget::update_constraints() noexcept
{
    _label_constraints = super::update_constraints();

    // Make room for button and margin.
    _button_size = {theme().size() * 2, theme().size()};
    hilet extra_size = extent2i{theme().margin<int>() + _button_size.width(), 0};

    auto r = max(_label_constraints + extra_size, _button_size);
    r.margins = theme().margin();
    r.alignment = *alignment;
    return r;
}

void toggle_widget::set_layout(widget_layout const& context) noexcept
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

        hilet button_square = aarectangle{
            narrow_cast<point2>(get<0>(_button_rectangle)),
            extent2{narrow_cast<float>(_button_rectangle.height()), narrow_cast<float>(_button_rectangle.height())}};

        _pip_circle = align(button_square, circle{theme().size() * 0.5f - 3.0f}, alignment::middle_center());

        hilet pip_to_button_margin_x2 = _button_rectangle.height() - narrow_cast<int>(_pip_circle.diameter());
        _pip_move_range = _button_rectangle.width() - narrow_cast<int>(_pip_circle.diameter()) - pip_to_button_margin_x2;
    }
    super::set_layout(context);
}

void toggle_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        draw_toggle_button(context);
        draw_toggle_pip(context);
        draw_button(context);
    }
}

void toggle_widget::draw_toggle_button(draw_context const& context) noexcept
{
    context.draw_box(
        layout(),
        _button_rectangle,
        background_color(),
        focus_color(),
        theme().border_width(),
        border_side::inside,
        corner_radii{_button_rectangle.height() * 0.5f});
}

void toggle_widget::draw_toggle_pip(draw_context const& context) noexcept
{
    _animated_value.update(state() == button_state::on ? 1.0f : 0.0f, context.display_time_point);
    if (_animated_value.is_animating()) {
        request_redraw();
    }

    hilet positioned_pip_circle = translate3{_pip_move_range * _animated_value.current_value(), 0.0f, 0.1f} * _pip_circle;

    hilet forground_color_ = state() == button_state::on ? accent_color() : foreground_color();
    context.draw_circle(layout(), positioned_pip_circle * 1.02f, forground_color_);
}

} // namespace hi::inline v1
