// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_button_widget.hpp"
#include "../GUI/gui_window.hpp"

namespace tt {

[[nodiscard]] float toolbar_button_widget::margin() const noexcept
{
    return 0.0f;
}

void toolbar_button_widget::constrain() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};
    constrain_button();

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    ttlet extra_size = extent2{theme().margin * 2.0f, theme().margin * 2.0f};
    _minimum_size += extra_size;
    _preferred_size += extra_size;
    _maximum_size += extra_size;

    tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
}

void toolbar_button_widget::set_layout(layout_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        if (_layout.store(context) >= layout_update::transform) {
            _label_rectangle = aarectangle{theme().margin, 0.0f, layout().width() - theme().margin * 2.0f, layout().height()};
        }
        layout_button(context);
    }
}

void toolbar_button_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, layout())) {
        draw_toolbar_button(context);
        draw_button(context);
    }
}

[[nodiscard]] bool toolbar_button_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());
    return is_toolbar(group) and enabled;
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

        default:;
        }
    }

    return super::handle_event(command);
}

void toolbar_button_widget::draw_toolbar_button(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    ttlet foreground_color_ = focus && window.active ? focus_color() : color::transparent();
    context.draw_box_with_border_inside(layout(), layout().rectangle(), background_color(), foreground_color_, corner_shapes{0.0f});
}

} // namespace tt
