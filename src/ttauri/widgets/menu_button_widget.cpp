// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "menu_button_widget.hpp"
#include "../text/font_book.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"
#include "../GUI/gui_window.hpp"

namespace tt::inline v1 {

widget_constraints const &menu_button_widget::set_constraints() noexcept
{
    _layout = {};

    // Make room for button and margin.
    _check_size = {theme().size, theme().size};
    _short_cut_size = {theme().size, theme().size};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    ttlet extra_size = extent2{theme().margin * 4.0f + _check_size.width() + _short_cut_size.width(), theme().margin * 2.0f};
    _constraints = set_constraints_button() + extra_size;
    _constraints.margins = 0.0f;
    return _constraints;
}

void menu_button_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        ttlet inside_rectangle = layout.rectangle() - theme().margin;

        _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_left());
        _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_right());

        _label_rectangle = aarectangle{
            _check_rectangle.right() + theme().margin, 0.0f, _short_cut_rectangle.left() - theme().margin, layout.height()};

        _check_glyph = font_book().find_glyph(elusive_icon::Ok);
        ttlet check_glyph_bb = _check_glyph.get_bounding_box();
        _check_glyph_rectangle = align(_check_rectangle, check_glyph_bb * theme().icon_size, alignment::middle_center());
    }
    set_layout_button(layout);
}

void menu_button_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        draw_menu_button(context);
        draw_check_mark(context);
        draw_button(context);
    }
}

[[nodiscard]] bool menu_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    return visible and enabled and any(group & tt::keyboard_focus_group::menu);
}

[[nodiscard]] bool menu_button_widget::handle_event(command command) noexcept
{
    tt_axiom(is_gui_thread());

    if (enabled) {
        switch (command) {
        case command::gui_menu_next:
            if (!is_last(keyboard_focus_group::menu)) {
                window.update_keyboard_target(keyboard_focus_group::menu, keyboard_focus_direction::forward);
                return true;
            }
            break;

        case command::gui_menu_prev:
            if (!is_first(keyboard_focus_group::menu)) {
                window.update_keyboard_target(keyboard_focus_group::menu, keyboard_focus_direction::backward);
                return true;
            }
            break;

        case command::gui_activate:
            activate();
            window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::forward);
            window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::backward);
            return true;

        default:;
        }
    }

    return super::handle_event(command);
}

void menu_button_widget::draw_menu_button(draw_context const &context) noexcept
{
    ttlet foreground_color_ = focus and active() ? focus_color() : color::transparent();
    context.draw_box(
        layout(), layout().rectangle(), background_color(), foreground_color_, theme().border_width, border_side::inside);
}

void menu_button_widget::draw_check_mark(draw_context const &context) noexcept
{
    auto state_ = state();

    // Checkmark or tristate.
    if (state_ == tt::button_state::on) {
        context.draw_glyph(layout(), translate_z(0.1f) * _check_glyph_rectangle, accent_color(), _check_glyph);
    }
}

} // namespace tt::inline v1
