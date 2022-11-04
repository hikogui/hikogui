// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_tab_button_widget.hpp"

namespace hi::inline v1 {

widget_constraints const& toolbar_tab_button_widget::set_constraints(set_constraints_context const& context) noexcept
{
    _layout = {};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    hilet extra_size = extent2{context.theme->margin * 2.0f, context.theme->margin};
    return _constraints = set_constraints_button(context) + extra_size;
}

void toolbar_tab_button_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        _label_rectangle = aarectangle{
            context.theme->margin,
            0.0f,
            context.width() - context.theme->margin * 2.0f,
            context.height() - context.theme->margin};
    }
    set_layout_button(context);
}

void toolbar_tab_button_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        draw_toolbar_tab_button(context);
        draw_button(context);
    }
}

[[nodiscard]] bool toolbar_tab_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::toolbar);
}

void toolbar_tab_button_widget::draw_toolbar_tab_button(draw_context const& context) noexcept
{
    // Draw the outline of the button across the clipping rectangle to clip the
    // bottom of the outline.
    hilet offset = layout().theme->margin + layout().theme->border_width;
    hilet outline_rectangle = aarectangle{0.0f, -offset, layout().width(), layout().height() + offset};

    // The focus line will be drawn by the parent widget (toolbar_widget) at 0.5.
    hilet button_z = *focus ? translate_z(0.6f) : translate_z(0.0f);

    // clang-format off
    auto button_color = (*hover or state() == button_state::on) ?
        layout().theme->color(semantic_color::fill, semantic_layer - 1) :
        layout().theme->color(semantic_color::fill, semantic_layer);
    // clang-format on

    hilet corner_radii = hi::corner_radii{0.0f, 0.0f, layout().theme->rounding_radius, layout().theme->rounding_radius};
    context.draw_box(
        layout(),
        button_z * outline_rectangle,
        button_color,
        *focus ? focus_color() : button_color,
        layout().theme->border_width,
        border_side::inside,
        corner_radii);
}

} // namespace hi::inline v1
