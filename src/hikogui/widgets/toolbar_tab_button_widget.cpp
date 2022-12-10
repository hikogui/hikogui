// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_tab_button_widget.hpp"

namespace hi::inline v1 {

[[nodiscard]] box_constraints toolbar_tab_button_widget::constraints() noexcept
{
    _layout = {};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    hilet extra_size = extent2{theme().margin * 2.0f, theme().margin};
    _label_constraints = constraints_button();
    return _label_constraints + extra_size;
}

void toolbar_tab_button_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet label_rectangle = aarectangle{
            theme().margin,
            0.0f,
            context.width() - theme().margin * 2.0f,
            context.height() - theme().margin};
        _label_shape = box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment};
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
    hilet offset = theme().margin + theme().border_width;
    hilet outline_rectangle = aarectangle{0.0f, -offset, narrow_cast<float>(layout().width()), narrow_cast<float>(layout().height()) + offset};

    // The focus line will be drawn by the parent widget (toolbar_widget) at 0.5.
    hilet button_z = *focus ? translate_z(0.6f) : translate_z(0.0f);

    // clang-format off
    auto button_color = (*hover or state() == button_state::on) ?
        theme().color(semantic_color::fill, semantic_layer - 1) :
        theme().color(semantic_color::fill, semantic_layer);
    // clang-format on

    hilet corner_radii = hi::corner_radii{0.0f, 0.0f, theme().rounding_radius, theme().rounding_radius};
    context.draw_box(
        layout(),
        button_z * outline_rectangle,
        button_color,
        *focus ? focus_color() : button_color,
        theme().border_width,
        border_side::inside,
        corner_radii);
}

} // namespace hi::inline v1
