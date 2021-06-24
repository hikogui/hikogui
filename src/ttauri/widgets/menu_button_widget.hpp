// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"

namespace tt {

class menu_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::callback_ptr_type;

    template<typename Label>
    menu_button_widget(gui_window &window, widget *parent, Label &&label, unique_or_borrow_ptr<delegate_type> delegate) noexcept :
        super(window, std::move(parent), std::move(delegate))
    {
        _margin = 0.0f;
        label_alignment = alignment::middle_left;
        set_label(std::forward<Label>(label));
    }

    template<typename Label, typename Value, typename... Args>
    requires(not std::is_convertible_v<Value,unique_or_borrow_ptr<delegate_type>>)
    menu_button_widget(gui_window &window, widget *parent, Label &&label, Value &&value, Args &&...args) noexcept :
        menu_button_widget(
            window,
            parent,
            std::forward<Label>(label),
            make_unique_default_button_delegate<button_type::radio>(std::forward<Value>(value), std::forward<Args>(args)...))
    {
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            // Make room for button and margin.
            _check_size = {theme::global().size, theme::global().size};
            _short_cut_size = {theme::global().size, theme::global().size};

            // On left side a check mark, on right side short-cut. Around the label extra margin.
            ttlet extra_size = extent2{
                theme::global().margin * 4.0f + _check_size.width() + _short_cut_size.width(), theme::global().margin * 2.0f};
            _minimum_size += extra_size;
            _preferred_size += extra_size;
            _maximum_size += extra_size;

            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_axiom(is_gui_thread());

        need_layout |= _request_relayout.exchange(false);
        if (need_layout) {
            ttlet inside_rectangle = shrink(rectangle(), theme::global().margin);

            _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_left);
            _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_right);

            _label_rectangle = aarectangle{
                _check_rectangle.right() + theme::global().margin,
                0.0f,
                _short_cut_rectangle.left() - theme::global().margin,
                height()};

            _check_glyph = to_font_glyph_ids(elusive_icon::Ok);
            ttlet check_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_check_glyph);
            _check_glyph_rectangle =
                align(_check_rectangle, scale(check_glyph_bb, theme::global().icon_size), alignment::middle_center);
        }
        super::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (overlaps(context, _clipping_rectangle)) {
            draw_menu_button(context);
            draw_check_mark(context);
        }

        super::draw(std::move(context), display_time_point);
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept
    {
        tt_axiom(is_gui_thread());
        return is_menu(group) and enabled;
    }

    [[nodiscard]] bool handle_event(command command) noexcept
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

private:
    font_glyph_ids _check_glyph;
    extent2 _check_size;
    aarectangle _check_rectangle;
    aarectangle _check_glyph_rectangle;
    extent2 _short_cut_size;
    aarectangle _short_cut_rectangle;

    void draw_menu_button(draw_context const &context) noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet foreground_color_ = _focus && window.active ? focus_color() : color::transparent();
        context.draw_box_with_border_inside(rectangle(), background_color(), foreground_color_, corner_shapes{0.0f});
    }

    void draw_check_mark(draw_context const &context) noexcept
    {
        tt_axiom(is_gui_thread());

        auto state_ = state();

        // Checkmark or tristate.
        if (state_ == tt::button_state::on) {
            context.draw_glyph(_check_glyph, translate_z(0.1f) * _check_glyph_rectangle, accent_color());
        }
    }
};

} // namespace tt
