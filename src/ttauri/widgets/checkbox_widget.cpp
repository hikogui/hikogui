// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "checkbox_widget.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"

namespace tt {

checkbox_widget::checkbox_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept :
    super(window, parent, std::move(delegate))
{
    label_alignment = alignment::top_left;
}

checkbox_widget::checkbox_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
    checkbox_widget(window, parent, weak_or_unique_ptr<delegate_type>{delegate})
{
}

[[nodiscard]] bool checkbox_widget::constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::constrain(display_time_point, need_reconstrain)) {
        // Make room for button and margin.
        _button_size = {theme().size, theme().size};
        ttlet extra_size = extent2{theme().margin + _button_size.width(), 0.0f};
        _minimum_size += extra_size;
        _preferred_size += extra_size;
        _maximum_size += extra_size;

        _minimum_size = max(_minimum_size, _button_size);
        _preferred_size = max(_minimum_size, _button_size);
        _maximum_size = max(_minimum_size, _button_size);

        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void checkbox_widget::layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _request_layout.exchange(false);
    if (need_layout) {
        _button_rectangle = align(rectangle(), _button_size, alignment::top_left);

        _label_rectangle = aarectangle{_button_rectangle.right() + theme().margin, 0.0f, width(), height()};

        _check_glyph = to_font_glyph_ids(font_book(), elusive_icon::Ok);
        ttlet check_glyph_bb = _check_glyph.get_bounding_box();
        _check_glyph_rectangle =
            align(_button_rectangle, scale(check_glyph_bb, theme().icon_size), alignment::middle_center);

        _minus_glyph = to_font_glyph_ids(font_book(), elusive_icon::Minus);
        ttlet minus_glyph_bb = _minus_glyph.get_bounding_box();
        _minus_glyph_rectangle =
            align(_button_rectangle, scale(minus_glyph_bb, theme().icon_size), alignment::middle_center);
    }
    super::layout(displayTimePoint, need_layout);
}

void checkbox_widget::draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    if (overlaps(context, _clipping_rectangle)) {
        draw_check_box(context);
        draw_check_mark(context);
    }

    super::draw(std::move(context), display_time_point);
}

void checkbox_widget::draw_check_box(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());
    context.draw_box_with_border_inside(_button_rectangle, background_color(), focus_color());
}

void checkbox_widget::draw_check_mark(draw_context context) noexcept
{
    tt_axiom(is_gui_thread());

    auto state_ = state();

    // Checkmark or tristate.
    if (state_ == tt::button_state::on) {
        context.draw_glyph(_check_glyph, theme().icon_size, translate_z(0.1f) * _check_glyph_rectangle, accent_color());

    } else if (state_ == tt::button_state::off) {
        ;

    } else {
        context.draw_glyph(_minus_glyph, theme().icon_size, translate_z(0.1f) * _minus_glyph_rectangle, accent_color());
    }
}

} // namespace tt
