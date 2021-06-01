// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "../stencils/label_stencil.hpp"
#include "../GUI/draw_context.hpp"
#include "../text/font_book.hpp"
#include "../observable.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

/** A checkbox widget.
 *
 * @tparam T The type of the value to monitor/modify
 */
template<typename T>
class checkbox_widget : public abstract_button_widget<T,button_type::toggle> {
public:
    using super = abstract_button_widget<T,button_type::toggle>;
    using value_type = typename super::value_type;
    using delegate_type = typename super::delegate_type;

    template<typename Value = value_type>
    checkbox_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>(),
        Value &&value = value_type{}) noexcept :
        super(window, std::move(parent), std::move(delegate), std::forward<Value>(value))
    {
    }

    template<typename Value>
    checkbox_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        Value &&value) noexcept :
        checkbox_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _label_stencil = stencil::make_unique(alignment::top_left, this->label(), theme::global->labelStyle);

            ttlet minimum_label_size = _label_stencil->minimum_size();
            ttlet preferred_label_size = _label_stencil->preferred_size();
            ttlet maximum_label_size = _label_stencil->maximum_size();

            this->_minimum_size = {
                minimum_label_size.width() + theme::global->smallSize + theme::global->margin,
                std::max(minimum_label_size.height(), theme::global->smallSize)};

            this->_preferred_size = {
                preferred_label_size.width() + theme::global->smallSize + theme::global->margin,
                std::max(preferred_label_size.height(), theme::global->smallSize)};

            this->_maximum_size = {
                maximum_label_size.width() + theme::global->smallSize + theme::global->margin,
                std::max(maximum_label_size.height(), theme::global->smallSize)};

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
            _checkbox_rectangle = aarectangle{
                0.0f,
                std::round(this->base_line() - theme::global->smallSize * 0.5f),
                theme::global->smallSize,
                theme::global->smallSize};

            ttlet label_x = _checkbox_rectangle.right() + theme::global->margin;
            _label_rectangle = aarectangle{label_x, 0.0f, this->rectangle().width() - label_x, this->rectangle().height()};
            _label_stencil->set_layout_parameters(_label_rectangle, this->base_line());

            _check_glyph = to_font_glyph_ids(elusive_icon::Ok);
            ttlet check_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_check_glyph);
            _check_glyph_rectangle =
                align(_checkbox_rectangle, scale(check_glyph_bb, theme::global->small_icon_size), alignment::middle_center);

            _minus_glyph = to_font_glyph_ids(elusive_icon::Minus);
            ttlet minus_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_minus_glyph);
            _minus_glyph_rectangle =
                align(_checkbox_rectangle, scale(minus_glyph_bb, theme::global->small_icon_size), alignment::middle_center);
        }

        super::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            draw_check_box(context);
            draw_check_mark(context);
            draw_label(context);
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    std::unique_ptr<label_stencil> _label_stencil;

    font_glyph_ids _check_glyph;
    aarectangle _check_glyph_rectangle;

    font_glyph_ids _minus_glyph;
    aarectangle _minus_glyph_rectangle;

    aarectangle _checkbox_rectangle;

    aarectangle _label_rectangle;

    void draw_check_box(draw_context const &context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_box_with_border_inside(_checkbox_rectangle, this->background_color(), this->focus_color());
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

    void draw_label(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_stencil_draw(_label_stencil, context, this->label_color());
    }
};

} // namespace tt
