// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "menu_button_widget.hpp"
#include "../text/font_book.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"
#include "../GUI/gui_window.hpp"

namespace hi::inline v1 {

widget_constraints const& menu_button_widget::set_constraints() noexcept
{
    _layout = {};

    // Make room for button and margin.
    _check_size = {theme().size, theme().size};
    _short_cut_size = {theme().size, theme().size};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    hilet extra_size = extent2{theme().margin * 4.0f + _check_size.width() + _short_cut_size.width(), theme().margin * 2.0f};
    _constraints = set_constraints_button() + extra_size;
    _constraints.margins = 0.0f;
    return _constraints;
}

void menu_button_widget::set_layout(widget_layout const& layout) noexcept
{
    if (compare_store(_layout, layout)) {
        hilet inside_rectangle = layout.rectangle() - theme().margin;

        if (layout.left_to_right()) {
            _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_left());
            _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_right());
            _label_rectangle = aarectangle{
                point2{_check_rectangle.right() + theme().margin, 0.0f},
                point2{_short_cut_rectangle.left() - theme().margin, layout.height()}};

        } else {
            _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_left());
            _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_right());
            _label_rectangle = aarectangle{
                point2{_short_cut_rectangle.right() + theme().margin, 0.0f},
                point2{_check_rectangle.left() - theme().margin, layout.height()}};
        }

        _check_glyph = font_book().find_glyph(elusive_icon::Ok);
        hilet check_glyph_bb = _check_glyph.get_bounding_box();
        _check_glyph_rectangle = align(_check_rectangle, check_glyph_bb * theme().icon_size, alignment::middle_center());
    }
    set_layout_button(layout);
}

void menu_button_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        draw_menu_button(context);
        draw_check_mark(context);
        draw_button(context);
    }
}

[[nodiscard]] bool menu_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    return *mode >= widget_mode::partial and any(group & hi::keyboard_focus_group::menu);
}

bool menu_button_widget::handle_event(gui_event const& event) noexcept
{
    switch (event.type()) {
    case gui_event_type::gui_menu_next:
        if (*mode >= widget_mode::partial and not is_last(keyboard_focus_group::menu)) {
            window.update_keyboard_target(keyboard_focus_group::menu, keyboard_focus_direction::forward);
            return true;
        }
        break;

    case gui_event_type::gui_menu_prev:
        if (*mode >= widget_mode::partial and not is_first(keyboard_focus_group::menu)) {
            window.update_keyboard_target(keyboard_focus_group::menu, keyboard_focus_direction::backward);
            return true;
        }
        break;

    case gui_event_type::gui_activate:
        if (*mode >= widget_mode::partial) {
            activate();
            window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::forward);
            window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::backward);
            return true;
        }
        break;

    default:;
    }

    return super::handle_event(event);
}

void menu_button_widget::draw_menu_button(draw_context const& context) noexcept
{
    hilet border_color = *focus ? focus_color() : color::transparent();
    context.draw_box(layout(), layout().rectangle(), background_color(), border_color, theme().border_width, border_side::inside);
}

void menu_button_widget::draw_check_mark(draw_context const& context) noexcept
{
    auto state_ = state();

    // Checkmark or tristate.
    if (state_ == hi::button_state::on) {
        context.draw_glyph(layout(), translate_z(0.1f) * _check_glyph_rectangle, accent_color(), _check_glyph);
    }
}

} // namespace hi::inline v1
