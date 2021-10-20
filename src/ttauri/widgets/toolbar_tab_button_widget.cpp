// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_tab_button_widget.hpp"
#include "../GUI/gui_window.hpp"

namespace tt {

[[nodiscard]] bool toolbar_tab_button_widget::constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept
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

void toolbar_tab_button_widget::layout(layout_context const &context, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    if (compare_then_assign(_layout, context) or need_layout) {
        _label_rectangle = aarectangle{theme().margin, 0.0f, width() - theme().margin * 2.0f, height() - theme().margin};

        layout_button(context, need_layout);
        request_redraw();
    }
}

void toolbar_tab_button_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, _layout)) {
        draw_toolbar_tab_button(context);
        draw_button(context);
    }
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

void toolbar_tab_button_widget::draw_toolbar_tab_button(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    // Draw the outline of the button across the clipping rectangle to clip the
    // bottom of the outline.
    ttlet offset = theme().margin + theme().border_width;
    ttlet outline_rectangle =
        aarectangle{rectangle().left(), rectangle().bottom() - offset, rectangle().width(), rectangle().height() + offset};

    // The focus line will be placed at 0.7.
    ttlet button_z = (focus && window.active) ? translate_z(0.8f) : translate_z(0.6f);

    auto button_color = (hover || state() == button_state::on) ? theme().color(theme_color::fill, semantic_layer - 1) :
                                                                 theme().color(theme_color::fill, semantic_layer);

    ttlet corner_shapes = tt::corner_shapes{0.0f, 0.0f, theme().rounding_radius, theme().rounding_radius};
    context.draw_box_with_border_inside(
        _layout,
        button_z * outline_rectangle,
        button_color,
        (focus && window.active) ? focus_color() : button_color,
        corner_shapes);
}

} // namespace tt
