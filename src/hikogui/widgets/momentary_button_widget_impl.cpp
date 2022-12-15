// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "momentary_button_widget.hpp"

namespace hi::inline v1 {

[[nodiscard]] box_constraints momentary_button_widget::update_constraints() noexcept
{
    _label_constraints = super::update_constraints();

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    hilet extra_size = extent2i{theme().margin<int>() * 2, theme().margin<int>() * 2};

    auto constraints = _label_constraints + extra_size;
    constraints.margins = theme().margin();
    return constraints;
}

void momentary_button_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet label_rectangle =
            aarectanglei{theme().margin<int>(), 0, context.width() - theme().margin<int>() * 2, context.height()};
        _on_label_shape = _off_label_shape = _other_label_shape =
            box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};
    }
    super::set_layout(context);
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
        theme().border_width(),
        border_side::inside,
        theme().rounding_radius());
}

} // namespace hi::inline v1
