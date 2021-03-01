// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_toggle_button_widget.hpp"
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
class checkbox_widget : public abstract_toggle_button_widget<T> {
public:
    using super = abstract_toggle_button_widget<T>;
    using value_type = typename super::value_type;

    observable<label> true_label;
    observable<label> false_label;
    observable<label> other_label;

    template<typename Value = observable<value_type>>
    checkbox_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        value_type true_value,
        value_type false_value,
        Value &&value = {}) noexcept :
        abstract_toggle_button_widget<T>(
            window,
            parent,
            std::move(true_value),
            std::move(false_value),
            std::forward<Value>(value))
    {
    }

    void init() noexcept override
    {
        _true_label_callback = true_label.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
        _false_label_callback = false_label.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
        _other_label_callback = other_label.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _true_label_stencil = stencil::make_unique(alignment::top_left, *true_label, theme::global->labelStyle);
            _false_label_stencil = stencil::make_unique(alignment::top_left, *false_label, theme::global->labelStyle);
            _other_label_stencil = stencil::make_unique(alignment::top_left, *other_label, theme::global->labelStyle);

            ttlet minimum_height = std::max(
                {_true_label_stencil->preferred_extent().height(),
                 _false_label_stencil->preferred_extent().height(),
                 _other_label_stencil->preferred_extent().height(),
                 theme::global->smallSize});

            ttlet minimum_width_of_labels = std::max(
                {_true_label_stencil->preferred_extent().width(),
                 _false_label_stencil->preferred_extent().width(),
                 _other_label_stencil->preferred_extent().width()});
            ttlet minimum_width = theme::global->smallSize + theme::global->margin + minimum_width_of_labels;

            this->_preferred_size = interval_extent2::make_minimum(minimum_width, minimum_height);

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
            _checkbox_rectangle = aarect{0.0f, std::round(this->base_line() - theme::global->smallSize * 0.5f), theme::global->smallSize, theme::global->smallSize};

            ttlet label_x = _checkbox_rectangle.p3().x() + theme::global->margin;
            _label_rectangle = aarect{label_x, 0.0f, this->rectangle().width() - label_x, this->rectangle().height()};
            _true_label_stencil->set_layout_parameters(_label_rectangle, this->base_line());
            _false_label_stencil->set_layout_parameters(_label_rectangle, this->base_line());
            _other_label_stencil->set_layout_parameters(_label_rectangle, this->base_line());

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
    typename decltype(true_label)::callback_ptr_type _true_label_callback;
    typename decltype(false_label)::callback_ptr_type _false_label_callback;
    typename decltype(other_label)::callback_ptr_type _other_label_callback;

    std::unique_ptr<label_stencil> _true_label_stencil;
    std::unique_ptr<label_stencil> _false_label_stencil;
    std::unique_ptr<label_stencil> _other_label_stencil;

    font_glyph_ids _check_glyph;
    aarect _check_glyph_rectangle;

    font_glyph_ids _minus_glyph;
    aarect _minus_glyph_rectangle;

    aarect _checkbox_rectangle;

    aarect _label_rectangle;

    void draw_check_box(draw_context const &context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_box_with_border_inside(_checkbox_rectangle, this->background_color(), this->focus_color());
    }

    void draw_check_mark(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        // Checkmark or tristate.
        if (this->value == this->true_value) {
            context.draw_glyph(_check_glyph, translate_z(0.1f) * _check_glyph_rectangle, this->accent_color());
        } else if (this->value == this->false_value) {
            ;
        } else {
            context.draw_glyph(_minus_glyph, translate_z(0.1f) * _minus_glyph_rectangle, this->accent_color());
        }
    }

    void draw_label(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet &labelCell = this->value == this->true_value ?
            _true_label_stencil :
            this->value == this->false_value ? _false_label_stencil : _other_label_stencil;

        labelCell->draw(context, this->label_color());
    }
};

} // namespace tt
