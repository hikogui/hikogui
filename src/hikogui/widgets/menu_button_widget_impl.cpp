// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "menu_button_widget.hpp"
#include "../font/module.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"

namespace hi::inline v1 {

[[nodiscard]] box_constraints menu_button_widget::update_constraints() noexcept
{
    _label_constraints = super::update_constraints();

    // Make room for button and margin.
    _check_size = {theme().size(), theme().size()};
    _short_cut_size = {theme().size(), theme().size()};

    // On left side a check mark, on right side short-cut. Around the label extra margin.
    hilet extra_size =
        extent2i{theme().margin<int>() * 4 + _check_size.width() + _short_cut_size.width(), theme().margin<int>() * 2};

    auto constraints = _label_constraints + extra_size;
    constraints.margins = 0;
    return constraints;
}

void menu_button_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet inside_rectangle = context.rectangle() - theme().margin<int>();

        if (os_settings::left_to_right()) {
            _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_left());
            _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_right());
            hilet label_rectangle = aarectanglei{
                point2i{_check_rectangle.right() + theme().margin<int>(), 0},
                point2i{_short_cut_rectangle.left() - theme().margin<int>(), context.height()}};
            _on_label_shape = _off_label_shape = _other_label_shape =
                box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};

        } else {
            _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_left());
            _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_right());
            hilet label_rectangle = aarectanglei{
                point2i{_short_cut_rectangle.right() + theme().margin<int>(), 0},
                point2i{_check_rectangle.left() - theme().margin<int>(), context.height()}};
            _on_label_shape = _off_label_shape = _other_label_shape =
                box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};
        }

        _check_glyph = find_glyph(elusive_icon::Ok);
        hilet check_glyph_bb = narrow_cast<aarectanglei>(_check_glyph.get_bounding_box() * theme().icon_size<float>());
        _check_glyph_rectangle = align(_check_rectangle, check_glyph_bb, alignment::middle_center());
    }

    super::set_layout(context);
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
    return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::menu);
}

bool menu_button_widget::handle_event(gui_event const& event) noexcept
{
    using enum gui_event_type;

    switch (event.type()) {
    case gui_menu_next:
        if (*mode >= widget_mode::partial and not is_last(keyboard_focus_group::menu)) {
            process_event(
                gui_event::window_set_keyboard_target(nullptr, keyboard_focus_group::menu, keyboard_focus_direction::forward));
            return true;
        }
        break;

    case gui_menu_prev:
        if (*mode >= widget_mode::partial and not is_first(keyboard_focus_group::menu)) {
            process_event(
                gui_event::window_set_keyboard_target(nullptr, keyboard_focus_group::menu, keyboard_focus_direction::backward));
            return true;
        }
        break;

    case gui_activate:
        if (*mode >= widget_mode::partial) {
            activate();
            process_event(
                gui_event::window_set_keyboard_target(nullptr, keyboard_focus_group::normal, keyboard_focus_direction::forward));
            process_event(
                gui_event::window_set_keyboard_target(nullptr, keyboard_focus_group::normal, keyboard_focus_direction::backward));
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
    context.draw_box(
        layout(), layout().rectangle(), background_color(), border_color, theme().border_width(), border_side::inside);
}

void menu_button_widget::draw_check_mark(draw_context const& context) noexcept
{
    auto state_ = state();

    // Checkmark or tristate.
    if (state_ == hi::button_state::on) {
        context.draw_glyph(
            layout(), translate_z(0.1f) * narrow_cast<aarectangle>(_check_glyph_rectangle), _check_glyph, accent_color());
    }
}

} // namespace hi::inline v1
