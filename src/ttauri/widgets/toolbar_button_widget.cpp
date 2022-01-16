// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_button_widget.hpp"
#include "../GUI/gui_window.hpp"

namespace tt::inline v1 {

widget_constraints const &toolbar_button_widget::set_constraints() noexcept
{
    _layout = {};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    ttlet extra_size = extent2{theme().margin * 2.0f, theme().margin * 2.0f};
    _constraints = set_constraints_button() + extra_size;
    _constraints.margins = 0.0f;
    return _constraints;
}

void toolbar_button_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        _label_rectangle = aarectangle{theme().margin, 0.0f, layout.width() - theme().margin * 2.0f, layout.height()};
    }
    set_layout_button(layout);
}

void toolbar_button_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        draw_toolbar_button(context);
        draw_button(context);
    }
}

[[nodiscard]] bool toolbar_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    return visible and enabled and any(group & tt::keyboard_focus_group::toolbar);
}

[[nodiscard]] bool toolbar_button_widget::handle_event(command command) noexcept
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

void toolbar_button_widget::draw_toolbar_button(draw_context const &context) noexcept
{
    ttlet foreground_color_ = focus and active() ? focus_color() : color::transparent();
    context.draw_box(
        layout(), layout().rectangle(), background_color(), foreground_color_, theme().border_width, border_side::inside);
}

} // namespace tt::inline v1
