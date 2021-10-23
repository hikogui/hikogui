// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "momentary_button_widget.hpp"

namespace tt {

void momentary_button_widget::constrain() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};
    constrain_button();

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    ttlet extra_size = extent2{theme().margin * 2.0f, theme().margin * 2.0f};
    _minimum_size += extra_size;
    _preferred_size += extra_size;
    _maximum_size += extra_size;

    tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
}

void momentary_button_widget::set_layout(layout_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        if (_layout.store(context) >= layout_update::transform) {
            _label_rectangle = aarectangle{theme().margin, 0.0f, layout().width() - theme().margin * 2.0f, layout().height()};
        }
        layout_button(context);
    }
}

void momentary_button_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, layout())) {
        draw_label_button(context);
        draw_button(context);
    }
}

void momentary_button_widget::draw_label_button(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    // Move the border of the button in the middle of a pixel.
    context.draw_box_with_border_inside(
        layout(), layout().rectangle(), background_color(), focus_color(), corner_shapes{theme().rounding_radius});
}

} // namespace tt
