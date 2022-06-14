// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "momentary_button_widget.hpp"

namespace hi::inline v1 {

widget_constraints const &momentary_button_widget::set_constraints() noexcept
{
    _layout = {};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    hilet extra_size = extent2{theme().margin * 2.0f, theme().margin * 2.0f};
    _constraints = set_constraints_button() + extra_size;
    _constraints.margins = theme().margin;
    return _constraints;
}

void momentary_button_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        _label_rectangle = aarectangle{theme().margin, 0.0f, layout.width() - theme().margin * 2.0f, layout.height()};
    }
    set_layout_button(layout);
}

void momentary_button_widget::draw(draw_context const &context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        draw_label_button(context);
        draw_button(context);
    }
}

void momentary_button_widget::draw_label_button(draw_context const &context) noexcept
{
    // Move the border of the button in the middle of a pixel.
    context.draw_box(
        layout(),
        layout().rectangle(),
        background_color(),
        focus_color(),
        theme().border_width,
        border_side::inside,
        corner_radii{theme().rounding_radius});
}

} // namespace hi::inline v1
