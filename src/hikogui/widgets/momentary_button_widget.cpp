// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "momentary_button_widget.hpp"

namespace hi::inline v1 {

[[nodiscard]] box_constraints momentary_button_widget::constraints(constraints_context const& context) noexcept
{
    _layout = {};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    hilet extra_size = extent2{context.theme->margin * 2.0f, context.theme->margin * 2.0f};
    _label_constraints = constraints_button(context);

    auto constraints = _label_constraints + extra_size;
    constraints.set_margins(narrow_cast<int>(context.theme->margin));
    return constraints;
}

void momentary_button_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet label_rectangle =
            aarectangle{context.theme->margin, 0.0f, narrow_cast<float>(context.width()) - context.theme->margin * 2.0f, narrow_cast<float>(context.height())};
        _label_shape = box_shape{_label_constraints, label_rectangle, context.theme->baseline_adjustment};
    }
    set_layout_button(context);
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
        layout().theme->border_width,
        border_side::inside,
        corner_radii{layout().theme->rounding_radius});
}

} // namespace hi::inline v1
