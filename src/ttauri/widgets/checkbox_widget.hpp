// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_toggle_button_widget.hpp"
#include "../cells/TextCell.hpp"
#include "../GUI/DrawContext.hpp"
#include "../text/FontBook.hpp"
#include "../text/format10.hpp"
#include "../observable.hpp"
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
    observable<std::u8string> true_label;
    observable<std::u8string> false_label;
    observable<std::u8string> other_label;

    template<typename Value = observable<T>>
    checkbox_widget(Window &window, Widget *parent, T true_value, T false_value, Value &&value = {}) noexcept :
        abstract_toggle_button_widget<T>(
            window,
            parent,
            std::move(true_value),
            std::move(false_value),
            std::forward<Value>(value))
    {
        _true_label_callback = scoped_callback(true_label, [this](auto...) {
            this->request_reconstrain = true;
        });
        _false_label_callback = scoped_callback(false_label, [this](auto...) {
            this->request_reconstrain = true;
        });
        _other_label_callback = scoped_callback(other_label, [this](auto...) {
            this->request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(this->mutex.is_locked_by_current_thread());

        if (Widget::update_constraints()) {
            _true_label_cell = std::make_unique<TextCell>(*true_label, theme->labelStyle);
            _false_label_cell = std::make_unique<TextCell>(*false_label, theme->labelStyle);
            _other_label_cell = std::make_unique<TextCell>(*other_label, theme->labelStyle);

            ttlet minimumHeight = std::max(
                {_true_label_cell->preferredExtent().height(),
                 _false_label_cell->preferredExtent().height(),
                 _other_label_cell->preferredExtent().height(),
                 Theme::smallSize});

            ttlet minimumWidthOfLabels = std::max(
                {_true_label_cell->preferredExtent().width(),
                 _false_label_cell->preferredExtent().width(),
                 _other_label_cell->preferredExtent().width()});
            ttlet minimumWidth = Theme::smallSize + Theme::margin + minimumWidthOfLabels;

            this->p_preferred_size = interval_vec2::make_minimum(minimumWidth, minimumHeight);
            this->p_preferred_base_line = relative_base_line{VerticalAlignment::Top, -Theme::smallSize * 0.5f};

            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_assume(this->mutex.is_locked_by_current_thread());

        need_layout |= std::exchange(this->request_relayout, false);
        if (need_layout) {
            _checkbox_rectangle = aarect{0.0f, this->base_line() - Theme::smallSize * 0.5f, Theme::smallSize, Theme::smallSize};

            ttlet labelX = _checkbox_rectangle.p3().x() + Theme::margin;
            _label_rectangle = aarect{labelX, 0.0f, this->rectangle().width() - labelX, this->rectangle().height()};

            _check_glyph = to_FontGlyphIDs(ElusiveIcon::Ok);
            ttlet checkGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(_check_glyph);
            _check_glyph_rectangle = align(_checkbox_rectangle, scale(checkGlyphBB, Theme::iconSize), Alignment::MiddleCenter);

            _minus_glyph = to_FontGlyphIDs(ElusiveIcon::Minus);
            ttlet minusGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(_minus_glyph);
            _minus_glyph_rectangle = align(_checkbox_rectangle, scale(minusGlyphBB, Theme::iconSize), Alignment::MiddleCenter);
        }

        return Widget::update_layout(displayTimePoint, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(this->mutex.is_locked_by_current_thread());

        draw_check_box(context);
        draw_check_mark(context);
        draw_label(context);
        Widget::draw(std::move(context), display_time_point);
    }

private:
    scoped_callback<decltype(true_label)> _true_label_callback;
    scoped_callback<decltype(false_label)> _false_label_callback;
    scoped_callback<decltype(other_label)> _other_label_callback;

    std::unique_ptr<TextCell> _true_label_cell;
    std::unique_ptr<TextCell> _false_label_cell;
    std::unique_ptr<TextCell> _other_label_cell;

    FontGlyphIDs _check_glyph;
    aarect _check_glyph_rectangle;

    FontGlyphIDs _minus_glyph;
    aarect _minus_glyph_rectangle;

    aarect _checkbox_rectangle;

    aarect _label_rectangle;

    void draw_check_box(DrawContext const &context) noexcept
    {
        tt_assume(this->mutex.is_locked_by_current_thread());

        context.drawBoxIncludeBorder(_checkbox_rectangle);
    }

    void draw_check_mark(DrawContext context) noexcept
    {
        tt_assume(this->mutex.is_locked_by_current_thread());

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
        tt_assume(this->mutex.is_locked_by_current_thread());

        if (*this->enabled) {
            context.color = theme->labelStyle.color;
        }

        ttlet &labelCell =
            this->value == this->true_value ? _true_label_cell : this->value == this->false_value ? _false_label_cell : _other_label_cell;

        labelCell->draw(context, _label_rectangle, Alignment::TopLeft, this->base_line(), true);
    }
};

} // namespace tt
