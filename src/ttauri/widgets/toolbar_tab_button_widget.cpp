// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_tab_button_widget.hpp"
#include "../GUI/gui_window.hpp"

namespace tt {

[[nodiscard]] bool
toolbar_tab_button_widget::constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::constrain(display_time_point, need_reconstrain)) {
        // On left side a check mark, on right side short-cut. Around the label extra margin.
        ttlet extra_size = extent2{theme().margin * 2.0f, theme().margin};
        _minimum_size += extra_size;
        _preferred_size += extra_size;
        _maximum_size += extra_size;

        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void
toolbar_tab_button_widget::layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _request_layout.exchange(false);
    if (need_layout) {
        _label_rectangle =
            aarectangle{theme().margin, 0.0f, width() - theme().margin * 2.0f, height() - theme().margin};
    }
    super::layout(displayTimePoint, need_layout);
}

void toolbar_tab_button_widget::draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    if (overlaps(context, _clipping_rectangle)) {
        draw_toolbar_tab_button(context);
    }
    draw_toolbar_tab_focus_line(context);

    super::draw(std::move(context), display_time_point);
}

void toolbar_tab_button_widget::request_redraw() const noexcept
{
    // A toolbar tab button draws a focus line across the whole toolbar
    // which is beyond it's own clipping rectangle. The parent is the toolbar
    // so it will include everything that needs to be redrawn.
    parent->request_redraw();
}

[[nodiscard]] bool toolbar_tab_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());
    return is_toolbar(group) and enabled;
}

[[nodiscard]] bool toolbar_tab_button_widget::handle_event(command command) noexcept
{
    tt_axiom(is_gui_thread());

    if (enabled) {
        switch (command) {
        case command::gui_toolbar_next:
            if (!is_last(keyboard_focus_group::toolbar)) {
                window.update_keyboard_target(keyboard_focus_group::toolbar, keyboard_focus_direction::forward);
                return true;
            }
            break;

        case command::gui_toolbar_prev:
            if (!is_first(keyboard_focus_group::toolbar)) {
                window.update_keyboard_target(keyboard_focus_group::toolbar, keyboard_focus_direction::backward);
                return true;
            }
            break;

        default:;
        }
    }

    return super::handle_event(command);
}

void toolbar_tab_button_widget::draw_toolbar_tab_focus_line(draw_context context) noexcept
{
    if (_focus and window.active and state() == tt::button_state::on) {
        ttlet parent_rectangle = aarectangle{_parent_to_local * parent->rectangle()};

        // Create a line, on the bottom of the toolbar over the full width.
        ttlet line_rectangle = aarectangle{
            parent_rectangle.left(), parent_rectangle.bottom(), parent_rectangle.width(), theme().border_width};

        context.set_clipping_rectangle(line_rectangle);

        if (overlaps(context, line_rectangle)) {
            // Draw the line above every other direct child of the toolbar, and between
            // the selected-tab (0.6) and unselected-tabs (0.8).
            context.draw_filled_quad(translate_z(0.7f) * line_rectangle, focus_color());
        }
    }
}

void toolbar_tab_button_widget::draw_toolbar_tab_button(draw_context context) noexcept
{
    tt_axiom(is_gui_thread());

    // Override the clipping rectangle to match the toolbar rectangle exactly
    // so that the bottom border of the tab button is not drawn.
    context.set_clipping_rectangle(aarectangle{_parent_to_local * parent->clipping_rectangle()});

    ttlet offset = theme().margin + theme().border_width;
    ttlet outline_rectangle =
        aarectangle{rectangle().left(), rectangle().bottom() - offset, rectangle().width(), rectangle().height() + offset};

    // The focus line will be placed at 0.7.
    ttlet button_z = (_focus && window.active) ? translate_z(0.8f) : translate_z(0.6f);

    auto button_color = (_hover || state() == button_state::on) ? theme().color(theme_color::fill, semantic_layer - 1) :
                                                                  theme().color(theme_color::fill, semantic_layer);

    ttlet corner_shapes = tt::corner_shapes{0.0f, 0.0f, theme().rounding_radius, theme().rounding_radius};
    context.draw_box_with_border_inside(
        button_z * outline_rectangle, button_color, (_focus && window.active) ? focus_color() : button_color, corner_shapes);
}
}
