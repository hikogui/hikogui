// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "momentary_button_widget.hpp"

namespace tt {

[[nodiscard]] bool momentary_button_widget::constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::constrain(display_time_point, need_reconstrain)) {
        // On left side a check mark, on right side short-cut. Around the label extra margin.
        ttlet extra_size = extent2{theme().margin * 2.0f, theme().margin * 2.0f};
        _minimum_size += extra_size;
        _preferred_size += extra_size;
        _maximum_size += extra_size;

        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

void momentary_button_widget::layout(
    matrix3 const &to_window,
    extent2 const &new_size,
    utc_nanoseconds display_time_point,
    bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    if (set_layout(to_window, new_size) or need_layout) {
        _label_rectangle = aarectangle{theme().margin, 0.0f, width() - theme().margin * 2.0f, height()};

        layout_button(to_window, display_time_point, need_layout);
        request_redraw();
    }
}

void momentary_button_widget::draw(draw_context context, utc_nanoseconds display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    if (overlaps(context, _clipping_rectangle)) {
        context.set_clipping_rectangle(_clipping_rectangle);
        draw_label_button(context);
    }

    super::draw(std::move(context), display_time_point);
}

void momentary_button_widget::draw_label_button(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    // Move the border of the button in the middle of a pixel.
    context.draw_box_with_border_inside(rectangle(), background_color(), focus_color(), corner_shapes{theme().rounding_radius});
}

} // namespace tt
