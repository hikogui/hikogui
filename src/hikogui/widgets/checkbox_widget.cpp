// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "checkbox_widget.hpp"
#include "../text/font_book.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"

namespace hi::inline v1 {

widget_constraints const& checkbox_widget::set_constraints() noexcept
{
    _layout = {};
    _button_size = {theme().size, theme().size};
    hilet extra_size = extent2{theme().margin + _button_size.width(), 0.0f};

    _constraints = max(set_constraints_button() + extra_size, _button_size);
    _constraints.margins = theme().margin;
    _constraints.baseline = widget_baseline{0.5f, alignment->vertical(), theme().cap_height, theme().size}; 
    return _constraints;
}

void checkbox_widget::set_layout(widget_layout const& layout) noexcept
{
    if (compare_store(_layout, layout)) {
        auto alignment_ = layout.left_to_right() ? *alignment : mirror(*alignment);

        if (alignment_ == horizontal_alignment::left or alignment_ == horizontal_alignment::right) {
            _button_rectangle = round(align(layout.rectangle(), _button_size, alignment_));
        } else {
            hi_not_implemented();
        }

        hilet label_width = layout.width() - (_button_rectangle.width() + theme().margin);
        if (alignment_ == horizontal_alignment::left) {
            hilet label_left = _button_rectangle.right() + theme().margin;
            _label_rectangle = aarectangle{label_left, 0.0f, label_width, layout.height()};

        } else if (alignment_ == horizontal_alignment::right) {
            _label_rectangle = aarectangle{0.0f, 0.0f, label_width, layout.height()};
        } else {
            hi_not_implemented();
        }

        _check_glyph = font_book().find_glyph(elusive_icon::Ok);
        hilet check_glyph_bb = _check_glyph.get_bounding_box();
        _check_glyph_rectangle = align(_button_rectangle, check_glyph_bb * theme().icon_size, alignment::middle_center());

        _minus_glyph = font_book().find_glyph(elusive_icon::Minus);
        hilet minus_glyph_bb = _minus_glyph.get_bounding_box();
        _minus_glyph_rectangle = align(_button_rectangle, minus_glyph_bb * theme().icon_size, alignment::middle_center());
    }
    set_layout_button(layout);
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
    context.draw_box(layout(), _button_rectangle, background_color(), focus_color(), theme().border_width, border_side::inside);
}

void checkbox_widget::draw_check_mark(draw_context const& context) noexcept
{
    auto state_ = state();

    // Checkmark or tristate.
    if (state_ == hi::button_state::on) {
        context.draw_glyph(layout(), translate_z(0.1f) * _check_glyph_rectangle, accent_color(), _check_glyph);

    } else if (state_ == hi::button_state::off) {
        ;

    } else {
        context.draw_glyph(layout(), translate_z(0.1f) * _minus_glyph_rectangle, accent_color(), _minus_glyph);
    }
}

} // namespace hi::inline v1
