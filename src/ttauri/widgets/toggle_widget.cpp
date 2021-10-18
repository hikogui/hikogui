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

[[nodiscard]] bool
toggle_widget::constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::constrain(display_time_point, need_reconstrain)) {
        // Make room for button and margin.
        _button_size = {theme().size * 2.0f, theme().size};
        ttlet extra_size = extent2{theme().margin + _button_size.width(), 0.0f};
        _minimum_size += extra_size;
        _preferred_size += extra_size;
        _maximum_size += extra_size;

        _minimum_size = max(_minimum_size, _button_size);
        _preferred_size = max(_minimum_size, _button_size);
        _maximum_size = max(_minimum_size, _button_size);

        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void toggle_widget::layout(utc_nanoseconds displayTimePoint, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _relayout.exchange(false);
    if (need_layout) {
        _button_rectangle = align(rectangle(), _button_size, alignment::top_left);

        _label_rectangle = aarectangle{_button_rectangle.right() + theme().margin, 0.0f, width(), height()};

        ttlet button_square =
            aarectangle{get<0>(_button_rectangle), extent2{_button_rectangle.height(), _button_rectangle.height()}};

        _pip_rectangle =
            align(button_square, extent2{theme().icon_size, theme().icon_size}, alignment::middle_center);

        ttlet pip_to_button_margin_x2 = _button_rectangle.height() - _pip_rectangle.height();
        _pip_move_range = _button_rectangle.width() - _pip_rectangle.width() - pip_to_button_margin_x2;
    }
    super::layout(displayTimePoint, need_layout);
}

void toggle_widget::draw(draw_context context, utc_nanoseconds display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    if (overlaps(context, _clipping_rectangle)) {
        draw_toggle_button(context);
        draw_toggle_pip(context, display_time_point);
    }

    super::draw(std::move(context), display_time_point);
}


void toggle_widget::draw_toggle_button(draw_context context) noexcept
{
    tt_axiom(is_gui_thread());

    context.draw_box_with_border_inside(
        _button_rectangle, background_color(), focus_color(), corner_shapes{_button_rectangle.height() * 0.5f});
}

void toggle_widget::draw_toggle_pip(draw_context draw_context, utc_nanoseconds display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    _animated_value.update(state() == button_state::on ? 1.0f : 0.0f, display_time_point);
    if (_animated_value.is_animating()) {
        request_redraw();
    }

    ttlet positioned_pip_rectangle = translate3{_pip_move_range * _animated_value.current_value(), 0.0f, 0.1f} * _pip_rectangle;

    ttlet forground_color_ = state() == button_state::on ? accent_color() : foreground_color();
    draw_context.draw_box(positioned_pip_rectangle, forground_color_, corner_shapes{positioned_pip_rectangle.height() * 0.5f});
}

}
