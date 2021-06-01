// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "button_delegate.hpp"
#include "label_widget.hpp"
#include "button_shape.hpp"
#include "button_type.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename T, button_shape ButtonShape, button_type ButtonType>
class button_widget final : public abstract_button_widget<T, ButtonType> {
public:
    static constexpr button_shape button_shape = ButtonShape;

    using super = abstract_button_widget<T, ButtonType>;
    using value_type = T;
    using delegate_type = typename button_delegate<value_type, ButtonType>;
    using callback_ptr_type = typename delegate_type::pressed_callback_ptr_type;

    button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>()) noexcept :
        super(window, std::move(parent), std::move(delegate))
    {
    }

    template<typename Value>
    button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate,
        Value &&value) noexcept :
        button_widget(window, std::move(parent), std::move(delegate))
    {
        set_value(std::forward<Value>(value));
    }

    template<typename Value>
    button_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        button_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }

    void init() noexcept override
    {
        ttlet alignment = button_shape == button_shape::label ? tt::alignment::middle_center : tt::alignment::top_left;

        _on_label_widget = this->make_widget<label_widget>(this->delegate_ptr<label_delegate>(), alignment);
        _off_label_widget = this->make_widget<label_widget>(this->delegate_ptr<label_delegate>(), alignment);
        _other_label_widget = this->make_widget<label_widget>(this->delegate_ptr<label_delegate>(), alignment);

        _on_label_widget->id = "on_label";
        _off_label_widget->id = "off_label";
        _other_label_widget->id = "other_label";
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            auto extra_size = extent2{};
            switch (button_shape) {
            case button_shape::label: extra_size = theme::global->margin2Dx2; break;
            case button_shape::checkbox: [[fallthrough]];
            case button_shape::radio: extra_size = extent2{theme::global->smallSize + theme::global->margin, 0.0f}; break;
            case button_shape::toggle: extra_size = extent2{theme::global->smallSize * 2.0f + theme::global->margin, 0.0f}; break;
            default: tt_no_default();
            }

            this->_minimum_size = _on_label_widget->minimum_size();
            this->_preferred_size = _on_label_widget->preferred_size();
            this->_maximum_size = _on_label_widget->maximum_size();

            this->_minimum_size = max(this->_minimum_size, _off_label_widget->minimum_size());
            this->_preferred_size = max(this->_preferred_size, _off_label_widget->preferred_size());
            this->_maximum_size = max(this->_maximum_size, _off_label_widget->maximum_size());

            this->_minimum_size = max(this->_minimum_size, _other_label_widget->minimum_size());
            this->_preferred_size = max(this->_preferred_size, _other_label_widget->preferred_size());
            this->_maximum_size = max(this->_maximum_size, _other_label_widget->maximum_size());

            // Make room for button and margin.
            this->_minimum_size += extra_size;
            this->_preferred_size += extra_size;
            this->_maximum_size += extra_size;

            // Make sure the widget is at least smallSize.
            this->_minimum_size = max(this->_minimum_size, theme::global->smallSize2D);
            this->_preferred_size = max(this->_minimum_size, theme::global->smallSize2D);
            this->_maximum_size = max(this->_minimum_size, theme::global->smallSize2D);

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
            ttlet inner_margin = theme::global->margin;

            auto button_size = extent2{};
            switch (button_shape) {
            case button_shape::label: button_size = this->size(); break;
            case button_shape::checkbox: [[fallthrough]];
            case button_shape::radio: button_size = extent2{theme::global->smallSize, theme::global->smallSize}; break;
            case button_shape::toggle: button_size = extent2{theme::global->smallSize * 2.0f, theme::global->smallSize}; break;
            default: tt_no_default();
            }

            auto label_rect = button_shape == button_shape::label ?
                aarectangle{inner_margin, inner_margin, this->width() - inner_margin * 2, this->height() - inner_margin * 2} :
                aarectangle{
                    button_size.width() + inner_margin, 0.0f, this->width() - button_size.width() - inner_margin, this->height()};

            _on_label_widget->set_layout_parameters_from_parent(label_rect);
            _off_label_widget->set_layout_parameters_from_parent(label_rect);
            _other_label_widget->set_layout_parameters_from_parent(label_rect);

            _button_rect = aarectangle{point2{0.0f, this->height() - button_size.height()}, button_size};

            _check_glyph = to_font_glyph_ids(elusive_icon::Ok);
            ttlet check_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_check_glyph);
            _check_glyph_rectangle =
                align(_button_rect, scale(check_glyph_bb, theme::global->small_icon_size), alignment::middle_center);

            _minus_glyph = to_font_glyph_ids(elusive_icon::Minus);
            ttlet minus_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_minus_glyph);
            _minus_glyph_rectangle =
                align(_button_rect, scale(minus_glyph_bb, theme::global->small_icon_size), alignment::middle_center);
        }
        super::update_layout(displayTimePoint, need_layout);
    }

    //[[nodiscard]] color background_color() const noexcept override
    //{
    //    if (this->state() == button_state::on) {
    //        return this->accent_color();
    //    } else {
    //        return super::background_color();
    //    }
    //}

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            switch (button_shape) {
            case button_shape::label: draw_label_button(context, display_time_point); break;
            case button_shape::checkbox:
                draw_check_box(context);
                draw_check_mark(context);
                break;
            default: tt_no_default();
            }
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    std::shared_ptr<label_widget> _on_label_widget;
    std::shared_ptr<label_widget> _off_label_widget;
    std::shared_ptr<label_widget> _other_label_widget;
    aarectangle _button_rect;
    bool _pressed;

    font_glyph_ids _check_glyph;
    aarectangle _check_glyph_rectangle;

    font_glyph_ids _minus_glyph;
    aarectangle _minus_glyph_rectangle;

    void draw_label_button(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
    {
        // Move the border of the button in the middle of a pixel.
        context.draw_box_with_border_inside(
            _button_rect, this->background_color(), this->focus_color(), corner_shapes{theme::global->roundingRadius});
    }

    void draw_check_box(draw_context const &context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_box_with_border_inside(_button_rect, this->background_color(), this->focus_color());
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

template<typename T>
using label_button_widget = button_widget<T, button_shape::label, button_type::momentary>;

template<typename T>
using checkbox_widget = button_widget<T, button_shape::checkbox, button_type::toggle>;

} // namespace tt
