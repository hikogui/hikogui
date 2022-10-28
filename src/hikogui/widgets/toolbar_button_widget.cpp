// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_button_widget.hpp"

namespace hi::inline v1 {

widget_constraints const& toolbar_button_widget::set_constraints(set_constraints_context const &context) noexcept
{
    _layout = {};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    hilet extra_size = extent2{context.theme->margin * 2.0f, context.theme->margin * 2.0f};
    _constraints = set_constraints_button(context) + extra_size;
    _constraints.margins = 0.0f;
    return _constraints;
}

void toolbar_button_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        _label_rectangle =
            aarectangle{context.theme->margin, 0.0f, context.width() - context.theme->margin * 2.0f, context.height()};
    }
    set_layout_button(context);
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
        layout(), layout().rectangle(), background_color(), border_color, layout().theme->border_width, border_side::inside);
}

} // namespace hi::inline v1
