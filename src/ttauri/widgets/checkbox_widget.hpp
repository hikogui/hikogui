// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"

namespace tt {

template<typename T>
class checkbox_widget final : public abstract_button_widget<T, button_type::toggle> {
public:
    using super = abstract_button_widget<T, button_type::toggle>;
    using value_type = typename super::value_type;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::pressed_callback_ptr_type;

    checkbox_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>()) noexcept :
        super(window, std::move(parent), std::move(delegate))
    {
        this->_label_alignment = alignment::top_left;
    }

    template<typename Value>
    checkbox_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate,
        Value &&value) noexcept :
        checkbox_widget(window, std::move(parent), std::move(delegate))
    {
        this->set_value(std::forward<Value>(value));
    }

    template<typename Value>
    checkbox_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        checkbox_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            // Make room for button and margin.
            this->_button_size = {theme::global->smallSize, theme::global->smallSize};
            ttlet extra_size = extent2{theme::global->margin + this->_button_size.width(), 0.0f};
            this->_minimum_size += extra_size;
            this->_preferred_size += extra_size;
            this->_maximum_size += extra_size;

            // Make sure the widget is at least smallSize.
            this->_minimum_size = max(this->_minimum_size, this->_button_size);
            this->_preferred_size = max(this->_minimum_size, this->_button_size);
            this->_maximum_size = max(this->_minimum_size, this->_button_size);

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
            this->_button_rectangle = align(this->rectangle(), this->_button_size, alignment::top_left);

            ttlet label_rectangle =
                aarectangle{this->_button_rectangle.right() + theme::global->margin, 0.0f, this->width(), this->height()};

            this->_on_label_widget->set_layout_parameters_from_parent(label_rectangle);
            this->_off_label_widget->set_layout_parameters_from_parent(label_rectangle);
            this->_other_label_widget->set_layout_parameters_from_parent(label_rectangle);

            _check_glyph = to_font_glyph_ids(elusive_icon::Ok);
            ttlet check_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_check_glyph);
            _check_glyph_rectangle =
                align(this->_button_rectangle, scale(check_glyph_bb, theme::global->small_icon_size), alignment::middle_center);

            _minus_glyph = to_font_glyph_ids(elusive_icon::Minus);
            ttlet minus_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_minus_glyph);
            _minus_glyph_rectangle =
                align(this->_button_rectangle, scale(minus_glyph_bb, theme::global->small_icon_size), alignment::middle_center);
        }
        widget::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            draw_check_box(context);
            draw_check_mark(context);
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    extent2 _button_size;
    aarectangle _button_rectangle;
    font_glyph_ids _check_glyph;
    aarectangle _check_glyph_rectangle;
    font_glyph_ids _minus_glyph;
    aarectangle _minus_glyph_rectangle;

    void draw_check_box(draw_context const &context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        context.draw_box_with_border_inside(this->_button_rectangle, this->background_color(), this->focus_color());
    }

    void draw_check_mark(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto state_ = this->state();

        // Checkmark or tristate.
        if (state_ == tt::button_state::on) {
            context.draw_glyph(_check_glyph, translate_z(0.1f) * _check_glyph_rectangle, this->accent_color());

        } else if (state_ == tt::button_state::off) {
            ;

        } else {
            context.draw_glyph(_minus_glyph, translate_z(0.1f) * _minus_glyph_rectangle, this->accent_color());
        }
    }
};

} // namespace tt
