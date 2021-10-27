// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "checkbox_widget.hpp"
#include "../text/font_book.hpp"
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

widget_constraints const &checkbox_widget::set_constraints() noexcept
{
    _layout = {};
    _button_size = {theme().size, theme().size};
    ttlet extra_size = extent2{theme().margin + _button_size.width(), 0.0f};

    _constraints = max(set_constraints_button() + extra_size, _button_size);
    _constraints.margin = theme().margin;
    return _constraints;
}

void checkbox_widget::set_layout(widget_layout const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        if (_layout.store(context) >= layout_update::transform) {
            _button_rectangle = align(layout().rectangle(), _button_size, alignment::top_left);
            _label_rectangle = aarectangle{_button_rectangle.right() + theme().margin, 0.0f, layout().width(), layout().height()};

            _check_glyph = font_book().find_glyph(elusive_icon::Ok);
            ttlet check_glyph_bb = _check_glyph.get_bounding_box();
            _check_glyph_rectangle = align(_button_rectangle, check_glyph_bb * theme().icon_size, alignment::middle_center);

            _minus_glyph = font_book().find_glyph(elusive_icon::Minus);
            ttlet minus_glyph_bb = _minus_glyph.get_bounding_box();
            _minus_glyph_rectangle = align(_button_rectangle, minus_glyph_bb * theme().icon_size, alignment::middle_center);
        }
        set_layout_button(context);
    }
}

void checkbox_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, layout())) {
        draw_check_box(context);
        draw_check_mark(context);
        draw_button(context);
    }
}

void checkbox_widget::draw_check_box(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());
    context.draw_box_with_border_inside(layout(), _button_rectangle, background_color(), focus_color());
}

void checkbox_widget::draw_check_mark(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    auto state_ = state();

    // Checkmark or tristate.
    if (state_ == tt::button_state::on) {
        context.draw_glyph(layout(), _check_glyph, translate_z(0.1f) * _check_glyph_rectangle, accent_color());

    } else if (state_ == tt::button_state::off) {
        ;

    } else {
        context.draw_glyph(layout(), _minus_glyph, translate_z(0.1f) * _minus_glyph_rectangle, accent_color());
    }
}

} // namespace tt
