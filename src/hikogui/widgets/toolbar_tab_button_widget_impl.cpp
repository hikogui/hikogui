// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_tab_button_widget.hpp"

namespace hi::inline v1 {

[[nodiscard]] box_constraints toolbar_tab_button_widget::update_constraints() noexcept
{
    _label_constraints = super::update_constraints();

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    hilet extra_size = extent2i{theme().margin<int>() * 2, theme().margin<int>()};
    return _label_constraints + extra_size;
}

void toolbar_tab_button_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet label_rectangle = aarectanglei{
            theme().margin<int>(), 0, context.width() - theme().margin<int>() * 2, context.height() - theme().margin<int>()};
        _on_label_shape = _off_label_shape = _other_label_shape =
            box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};
    }
    super::set_layout(context);
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
    hilet offset = theme().margin<int>() + theme().border_width();
    hilet outline_rectangle = aarectanglei{0, -offset, layout().width(), layout().height() + offset};

    // The focus line will be drawn by the parent widget (toolbar_widget) at 0.5.
    hilet button_z = *focus ? translate_z(0.6f) : translate_z(0.0f);

    // clang-format off
    auto button_color = (*hover or state() == button_state::on) ?
        theme().color(semantic_color::fill, semantic_layer - 1) :
        theme().color(semantic_color::fill, semantic_layer);
    // clang-format on

    hilet corner_radii = hi::corner_radii(
        0.0f, 0.0f, theme().rounding_radius<float>(), theme().rounding_radius<float>());

    context.draw_box(
        layout(),
        button_z * narrow_cast<aarectangle>(outline_rectangle),
        button_color,
        *focus ? focus_color() : button_color,
        theme().border_width(),
        border_side::inside,
        corner_radii);
}

} // namespace hi::inline v1
