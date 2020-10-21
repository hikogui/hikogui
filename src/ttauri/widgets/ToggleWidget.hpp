// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_bool_toggle_button_widget.hpp"
#include "../cells/TextCell.hpp"
#include "../GUI/DrawContext.hpp"
#include "../observable.hpp"
#include "../text/format10.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class ToggleWidget final : public abstract_bool_toggle_button_widget {
public:
    observable<std::u8string> onLabel;
    observable<std::u8string> offLabel;

    template<
        typename V = observable<bool>,
        typename L1 = observable<std::u8string>,
        typename L2 = observable<std::u8string>,
        typename L3 = observable<std::u8string>>
    ToggleWidget(
        Window &window,
        Widget *parent,
        V &&value = observable<bool>{},
        L1 &&onLabel = observable<std::u8string>{},
        L2 &&offLabel = observable<std::u8string>{}) noexcept :
        abstract_bool_toggle_button_widget(window, parent, std::forward<V>(value)),
        onLabel(std::forward<L1>(onLabel)),
        offLabel(std::forward<L2>(offLabel))
    {
        on_label_callback = scoped_callback(this->onLabel, [this](auto...) {
            request_reconstrain = true;
        });
        off_label_callback = scoped_callback(this->offLabel, [this](auto...) {
            request_reconstrain = true;
        });
    }

    ~ToggleWidget() {}

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (Widget::update_constraints()) {
            onLabelCell = std::make_unique<TextCell>(*onLabel, theme->labelStyle);
            offLabelCell = std::make_unique<TextCell>(*offLabel, theme->labelStyle);

            ttlet minimumHeight =
                std::max({onLabelCell->preferredExtent().height(), offLabelCell->preferredExtent().height(), Theme::smallSize});

            ttlet minimumWidth = std::max({onLabelCell->preferredExtent().width(), offLabelCell->preferredExtent().width()}) +
                Theme::smallSize * 2.0f + Theme::margin;

            p_preferred_size = interval_vec2::make_minimum(minimumWidth, minimumHeight);
            p_preferred_base_line = relative_base_line{VerticalAlignment::Top, -Theme::smallSize * 0.5f};

            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        need_layout |= std::exchange(request_relayout, false);
        if (need_layout) {
            toggleRectangle = aarect{
                -0.5f, // Expand horizontally due to rounded shape
                base_line() - Theme::smallSize * 0.5f,
                Theme::smallSize * 2.0f + 1.0f, // Expand horizontally due to rounded shape
                Theme::smallSize};

            ttlet labelX = Theme::smallSize * 2.0f + Theme::margin;
            labelRectangle = aarect{labelX, 0.0f, rectangle().width() - labelX, rectangle().height()};

            sliderRectangle = shrink(aarect{0.0f, toggleRectangle.y(), toggleRectangle.height(), toggleRectangle.height()}, 1.5f);

            ttlet sliderMoveWidth = Theme::smallSize * 2.0f - (sliderRectangle.x() * 2.0f);
            sliderMoveRange = sliderMoveWidth - sliderRectangle.width();
        }

        return Widget::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        drawToggle(context);
        drawSlider(context);
        drawLabel(context);
        Widget::draw(std::move(context), display_time_point);
    }

private:
    static constexpr hires_utc_clock::duration animationDuration = 150ms;

    aarect toggleRectangle;

    aarect sliderRectangle;
    float sliderMoveRange;

    aarect labelRectangle;

    std::unique_ptr<TextCell> onLabelCell;
    std::unique_ptr<TextCell> offLabelCell;
    std::unique_ptr<TextCell> _other_label_cell;

    scoped_callback<decltype(value)> value_callback;
    scoped_callback<decltype(onLabel)> on_label_callback;
    scoped_callback<decltype(offLabel)> off_label_callback;

    void drawToggle(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.cornerShapes = vec{toggleRectangle.height() * 0.5f};
        drawContext.drawBoxIncludeBorder(toggleRectangle);
    }

    void drawSlider(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        // Prepare animation values.
        ttlet animationProgress = value.animation_progress(animationDuration);
        if (animationProgress < 1.0f) {
            window.requestRedraw = true;
        }

        ttlet animatedValue = to_float(value, animationDuration);

        ttlet positionedSliderRectangle = mat::T2(sliderMoveRange * animatedValue, 0.0f) * sliderRectangle;

        if (*value) {
            if (*enabled && window.active) {
                drawContext.color = theme->accentColor;
            }
        } else {
            if (*enabled && window.active) {
                drawContext.color = hover ? theme->borderColor(p_semantic_layer + 1) : theme->borderColor(p_semantic_layer);
            }
        }
        std::swap(drawContext.color, drawContext.fillColor);
        drawContext.transform = mat::T{0.0f, 0.0f, 0.1f} * drawContext.transform;
        drawContext.cornerShapes = vec{positionedSliderRectangle.height() * 0.5f};
        drawContext.drawBoxIncludeBorder(positionedSliderRectangle);
    }

    void drawLabel(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        ttlet &labelCell = *value ? onLabelCell : offLabelCell;

        labelCell->draw(drawContext, labelRectangle, Alignment::TopLeft, base_line(), true);
    }
};

} // namespace tt
