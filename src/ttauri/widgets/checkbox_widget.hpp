// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_toggle_button_widget.hpp"
#include "../stencils/label_stencil.hpp"
#include "../GUI/DrawContext.hpp"
#include "../text/FontBook.hpp"
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
        Window &window,
        std::shared_ptr<widget> parent,
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

    void initialize() noexcept override
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

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (super::update_constraints()) {
            _true_label_stencil = stencil::make_unique(Alignment::TopLeft, *true_label, theme->labelStyle);
            _false_label_stencil = stencil::make_unique(Alignment::TopLeft, *false_label, theme->labelStyle);
            _other_label_stencil = stencil::make_unique(Alignment::TopLeft, *other_label, theme->labelStyle);

            ttlet minimum_height = std::max(
                {_true_label_stencil->preferred_extent().height(),
                 _false_label_stencil->preferred_extent().height(),
                 _other_label_stencil->preferred_extent().height(),
                 Theme::smallSize});

            ttlet minimum_width_of_labels = std::max(
                {_true_label_stencil->preferred_extent().width(),
                 _false_label_stencil->preferred_extent().width(),
                 _other_label_stencil->preferred_extent().width()});
            ttlet minimum_width = Theme::smallSize + Theme::margin + minimum_width_of_labels;

            this->_preferred_size = interval_vec2::make_minimum(minimum_width, minimum_height);
            this->_preferred_base_line = relative_base_line{VerticalAlignment::Top, -Theme::smallSize * 0.5f};

            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            _checkbox_rectangle = aarect{0.0f, this->base_line() - Theme::smallSize * 0.5f, Theme::smallSize, Theme::smallSize};

            ttlet label_x = _checkbox_rectangle.p3().x() + Theme::margin;
            _label_rectangle = aarect{label_x, 0.0f, this->rectangle().width() - label_x, this->rectangle().height()};
            _true_label_stencil->set_layout_parameters(_label_rectangle, this->base_line());
            _false_label_stencil->set_layout_parameters(_label_rectangle, this->base_line());
            _other_label_stencil->set_layout_parameters(_label_rectangle, this->base_line());

            _check_glyph = to_FontGlyphIDs(ElusiveIcon::Ok);
            ttlet check_glyph_bb = PipelineSDF::DeviceShared::getBoundingBox(_check_glyph);
            _check_glyph_rectangle = align(_checkbox_rectangle, scale(check_glyph_bb, Theme::iconSize), Alignment::MiddleCenter);

            _minus_glyph = to_FontGlyphIDs(ElusiveIcon::Minus);
            ttlet minus_glyph_bb = PipelineSDF::DeviceShared::getBoundingBox(_minus_glyph);
            _minus_glyph_rectangle = align(_checkbox_rectangle, scale(minus_glyph_bb, Theme::iconSize), Alignment::MiddleCenter);
        }

        return super::update_layout(displayTimePoint, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        draw_check_box(context);
        draw_check_mark(context);
        draw_label(context);
        super::draw(std::move(context), display_time_point);
    }

private:
    typename decltype(true_label)::callback_ptr_type _true_label_callback;
    typename decltype(false_label)::callback_ptr_type _false_label_callback;
    typename decltype(other_label)::callback_ptr_type _other_label_callback;

    std::unique_ptr<label_stencil> _true_label_stencil;
    std::unique_ptr<label_stencil> _false_label_stencil;
    std::unique_ptr<label_stencil> _other_label_stencil;

    FontGlyphIDs _check_glyph;
    aarect _check_glyph_rectangle;

    FontGlyphIDs _minus_glyph;
    aarect _minus_glyph_rectangle;

    aarect _checkbox_rectangle;

    aarect _label_rectangle;

    void draw_check_box(DrawContext const &context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        context.drawBoxIncludeBorder(_checkbox_rectangle);
    }

    void draw_check_mark(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        context.transform = mat::T{0.0, 0.0, 0.1f} * context.transform;

        if (*this->enabled && this->window.active) {
            context.color = theme->accentColor;
        }

        // Checkmark or tristate.
        if (this->value == this->true_value) {
            context.drawGlyph(_check_glyph, _check_glyph_rectangle);
        } else if (this->value == this->false_value) {
            ;
        } else {
            context.drawGlyph(_minus_glyph, _minus_glyph_rectangle);
        }
    }

    void draw_label(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (*this->enabled) {
            context.color = theme->labelStyle.color;
        }

        ttlet &labelCell =
            this->value == this->true_value ? _true_label_stencil : this->value == this->false_value ? _false_label_stencil : _other_label_stencil;

        labelCell->draw(context, true);
    }
};

} // namespace tt
