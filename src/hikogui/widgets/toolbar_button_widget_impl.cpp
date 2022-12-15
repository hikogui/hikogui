// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_button_widget.hpp"

namespace hi::inline v1 {

[[nodiscard]] box_constraints toolbar_button_widget::update_constraints() noexcept
{
    _label_constraints = super::update_constraints();

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    hilet extra_size = extent2i{theme().margin<int>() * 2, theme().margin<int>() * 2};

    auto constraints = _label_constraints + extra_size;
    constraints.margins = 0;
    return constraints;
}

void toolbar_button_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet label_rectangle =
            aarectanglei{theme().margin<int>(), 0, context.width() - theme().margin<int>() * 2, context.height()};
        _on_label_shape = _off_label_shape = _other_label_shape =
            box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};
    }
    super::set_layout(context);
}

void toolbar_button_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        draw_toolbar_button(context);
        draw_button(context);
    }
}

[[nodiscard]] bool toolbar_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::toolbar);
}

void toolbar_button_widget::draw_toolbar_button(draw_context const& context) noexcept
{
    hilet border_color = *focus ? focus_color() : color::transparent();
    context.draw_box(
        layout(), layout().rectangle(), background_color(), border_color, theme().border_width(), border_side::inside);
}

} // namespace hi::inline v1
