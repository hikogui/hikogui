// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "checkbox_widget.hpp"
#include "../font/module.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"

namespace hi::inline v1 {

[[nodiscard]] box_constraints checkbox_widget::update_constraints() noexcept
{
    _label_constraints = super::update_constraints();

    _button_size = {theme().size(), theme().size()};
    hilet extra_size = extent2i{theme().margin<int>() + _button_size.width(), 0};

    auto constraints = max(_label_constraints + extra_size, _button_size);
    constraints.margins = theme().margin();
    constraints.alignment = *alignment;
    return constraints;
}

void checkbox_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        auto alignment_ = os_settings::left_to_right() ? *alignment : mirror(*alignment);

        if (alignment_ == horizontal_alignment::left or alignment_ == horizontal_alignment::right) {
            _button_rectangle = align(context.rectangle(), _button_size, alignment_);
        } else {
            hi_not_implemented();
        }

        hilet label_width = context.width() - (_button_rectangle.width() + theme().margin<int>());
        if (alignment_ == horizontal_alignment::left) {
            hilet label_left = _button_rectangle.right() + theme().margin<int>();
            hilet label_rectangle = aarectanglei{label_left, 0, label_width, context.height()};
            _on_label_shape = _off_label_shape = _other_label_shape =
                box_shape(_label_constraints, label_rectangle, theme().baseline_adjustment());

        } else if (alignment_ == horizontal_alignment::right) {
            hilet label_rectangle = aarectanglei{0, 0, label_width, context.height()};
            _on_label_shape = _off_label_shape = _other_label_shape =
                box_shape(_label_constraints, label_rectangle, theme().baseline_adjustment());
        } else {
            hi_not_implemented();
        }

        _check_glyph = find_glyph(elusive_icon::Ok);
        hilet check_glyph_bb =
            narrow_cast<aarectanglei>(_check_glyph.get_bounding_box() * narrow_cast<float>(theme().icon_size()));
        _check_glyph_rectangle = align(_button_rectangle, check_glyph_bb, alignment::middle_center());

        _minus_glyph = find_glyph(elusive_icon::Minus);
        hilet minus_glyph_bb =
            narrow_cast<aarectanglei>(_minus_glyph.get_bounding_box() * narrow_cast<float>(theme().icon_size()));
        _minus_glyph_rectangle = align(_button_rectangle, minus_glyph_bb, alignment::middle_center());
    }
    super::set_layout(context);
}

void checkbox_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        draw_check_box(context);
        draw_check_mark(context);
        draw_button(context);
    }
}

void checkbox_widget::draw_check_box(draw_context const& context) noexcept
{
    context.draw_box(
        layout(),
        _button_rectangle,
        background_color(),
        focus_color(), theme().border_width(),
        border_side::inside);
}

void checkbox_widget::draw_check_mark(draw_context const& context) noexcept
{
    auto state_ = state();

    // Checkmark or tristate.
    if (state_ == hi::button_state::on) {
        context.draw_glyph(
            layout(), translate_z(0.1f) * narrow_cast<aarectangle>(_check_glyph_rectangle), _check_glyph, accent_color());

    } else if (state_ == hi::button_state::off) {
        ;

    } else {
        context.draw_glyph(
            layout(), translate_z(0.1f) * narrow_cast<aarectangle>(_minus_glyph_rectangle), _minus_glyph, accent_color());
    }
}

} // namespace hi::inline v1
