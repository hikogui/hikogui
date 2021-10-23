// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "menu_button_widget.hpp"
#include "../text/font_book.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"
#include "../GUI/gui_window.hpp"

namespace tt {

[[nodiscard]] float menu_button_widget::margin() const noexcept
{
    return 0.0f;
}

void menu_button_widget::constrain() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};
    constrain_button();

    // Make room for button and margin.
    _check_size = {theme().size, theme().size};
    _short_cut_size = {theme().size, theme().size};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    ttlet extra_size = extent2{theme().margin * 4.0f + _check_size.width() + _short_cut_size.width(), theme().margin * 2.0f};
    _minimum_size += extra_size;
    _preferred_size += extra_size;
    _maximum_size += extra_size;

    tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
}

void menu_button_widget::set_layout(layout_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        if (_layout.store(context) >= layout_update::transform) {
            ttlet inside_rectangle = layout().rectangle() - theme().margin;

            _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_left);
            _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_right);

            _label_rectangle = aarectangle{
                _check_rectangle.right() + theme().margin, 0.0f, _short_cut_rectangle.left() - theme().margin, layout().height()};

            _check_glyph = font_book().find_glyph(elusive_icon::Ok);
            ttlet check_glyph_bb = _check_glyph.get_bounding_box();
            _check_glyph_rectangle = align(_check_rectangle, check_glyph_bb * theme().icon_size, alignment::middle_center);
        }
        layout_button(context);
    }
}

void menu_button_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, layout())) {
        draw_menu_button(context);
        draw_check_mark(context);
        draw_button(context);
    }
}

[[nodiscard]] bool menu_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());
    return is_menu(group) and enabled;
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
    tt_axiom(is_gui_thread());

    ttlet foreground_color_ = focus && window.active ? focus_color() : color::transparent();
    context.draw_box_with_border_inside(layout(), layout().rectangle(), background_color(), foreground_color_, corner_shapes{0.0f});
}

void menu_button_widget::draw_check_mark(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    auto state_ = state();

    // Checkmark or tristate.
    if (state_ == tt::button_state::on) {
        context.draw_glyph(layout(), _check_glyph, theme().icon_size, translate_z(0.1f) * _check_glyph_rectangle, accent_color());
    }
}

} // namespace tt
