// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_tab_button_widget.hpp"
#include "../GUI/gui_window.hpp"

namespace tt::inline v1 {

widget_constraints const &toolbar_tab_button_widget::set_constraints() noexcept
{
    _layout = {};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    ttlet extra_size = extent2{theme().margin * 2.0f, theme().margin};
    return _constraints = set_constraints_button() + extra_size;
}

void toolbar_tab_button_widget::set_layout(widget_layout const &context) noexcept
{
    // The toolbar tab will draw below the button into the margin.
    auto context_ = context;
    context_.redraw_rectangle =
        aarectangle{translate2{0.0, -theme().margin} * get<0>(context.redraw_rectangle), get<3>(context.redraw_rectangle)};

    if (_layout.store(context_) >= layout_update::transform) {
        _label_rectangle =
            aarectangle{theme().margin, 0.0f, layout().width() - theme().margin * 2.0f, layout().height() - theme().margin};
    }
    set_layout_button(context_);
}

void toolbar_tab_button_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
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
        case command::gui_sysmenu_open: window.open_system_menu(); return true;

        default:;
        }
    }

    return super::handle_event(command);
}

void toolbar_tab_button_widget::draw_toolbar_tab_button(draw_context const &context) noexcept
{
    // Draw the outline of the button across the clipping rectangle to clip the
    // bottom of the outline.
    ttlet offset = theme().margin + theme().border_width;
    ttlet outline_rectangle = aarectangle{0.0f, -offset, layout().width(), layout().height() + offset};

    // The focus line will be placed at 0.7.
    ttlet button_z = (focus && window.active) ? translate_z(0.8f) : translate_z(0.6f);

    auto button_color = (hover || state() == button_state::on) ? theme().color(theme_color::fill, semantic_layer - 1) :
                                                                 theme().color(theme_color::fill, semantic_layer);

    ttlet corner_shapes = tt::corner_shapes{0.0f, 0.0f, theme().rounding_radius, theme().rounding_radius};
    context.draw_box(
        layout(),
        button_z * outline_rectangle,
        button_color,
        (focus && window.active) ? focus_color() : button_color,
        theme().border_width,
        border_side::inside,
        corner_shapes);
}

} // namespace tt::inline v1
