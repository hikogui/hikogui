// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"

namespace tt {

template<typename T>
class menu_button_widget final : public abstract_button_widget<T, button_type::momentary> {
public:
    using super = abstract_button_widget<T, button_type::momentary>;
    using value_type = typename super::value_type;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::pressed_callback_ptr_type;

    menu_button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>()) noexcept :
        super(window, std::move(parent), std::move(delegate))
    {
        this->_margin = 0.0f;
        this->_label_alignment = alignment::middle_left;
    }

    template<typename Value>
    menu_button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate,
        Value &&value) noexcept :
        menu_button_widget(window, std::move(parent), std::move(delegate))
    {
        this->set_value(std::forward<Value>(value));
    }

    template<typename Value>
    menu_button_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        menu_button_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            // Make room for button and margin.
            _check_size = {theme::global->smallSize, theme::global->smallSize};
            _short_cut_size = {theme::global->smallSize, theme::global->smallSize};

            // On left side a check mark, on right side short-cut. Around the label extra margin.
            ttlet extra_size = extent2{theme::global->margin * 2.0f + _check_size.width() + _short_cut_size.width(), 0.0f} +
                theme::global->margin2Dx2;
            this->_minimum_size += extra_size;
            this->_preferred_size += extra_size;
            this->_maximum_size += extra_size;

            tt_axiom(this->_minimum_size <= this->_preferred_size && this->_preferred_size <= this->_maximum_size);
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            ttlet inside_rectangle = shrink(this->rectangle(), theme::global->margin);

            _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_left);
            _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_right);

            ttlet label_rectangle = aarectangle{
                _check_rectangle.right() + theme::global->margin,
                0.0f,
                _short_cut_rectangle.left() - theme::global->margin,
                this->height()};

            this->_on_label_widget->set_layout_parameters_from_parent(label_rectangle);
            this->_off_label_widget->set_layout_parameters_from_parent(label_rectangle);
            this->_other_label_widget->set_layout_parameters_from_parent(label_rectangle);

            _check_glyph = to_font_glyph_ids(elusive_icon::Ok);
            ttlet check_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_check_glyph);
            _check_glyph_rectangle =
                align(_check_rectangle, scale(check_glyph_bb, theme::global->small_icon_size), alignment::middle_center);
        }
        widget::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            draw_menu_button(context);
            draw_check_mark(context);
        }

        super::draw(std::move(context), display_time_point);
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_menu(group) and this->enabled();
    }

    [[nodiscard]] bool handle_event(command command) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (this->enabled()) {
            switch (command) {
            case command::gui_menu_next:
                if (!this->is_last(keyboard_focus_group::menu)) {
                    this->window.update_keyboard_target(keyboard_focus_group::menu, keyboard_focus_direction::forward);
                    return true;
                }
                break;

            case command::gui_menu_prev:
                if (!this->is_first(keyboard_focus_group::menu)) {
                    this->window.update_keyboard_target(keyboard_focus_group::menu, keyboard_focus_direction::backward);
                    return true;
                }
                break;

            case command::gui_activate:
                this->delegate<delegate_type>().pressed(*this);
                this->window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::forward);
                this->window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::backward);
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
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet foreground_color_ = this->_focus && this->window.active ? this->focus_color() : color::transparent();
        context.draw_box_with_border_inside(this->rectangle(), this->background_color(), foreground_color_, corner_shapes{0.0f});
    }

    void draw_check_mark(draw_context const &context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto state_ = this->state();

        // Checkmark or tristate.
        if (state_ == tt::button_state::on) {
            context.draw_glyph(_check_glyph, translate_z(0.1f) * _check_glyph_rectangle, this->accent_color());
        }
    }
};

} // namespace tt
