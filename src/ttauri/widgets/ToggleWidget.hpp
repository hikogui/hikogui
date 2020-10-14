// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
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

class ToggleWidget final : public Widget {
public:
    observable<bool> value;
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
        Widget(window, parent),
        value(std::forward<V>(value)),
        onLabel(std::forward<L1>(onLabel)),
        offLabel(std::forward<L2>(offLabel))
    {
        [[maybe_unused]] ttlet value_cbid = this->value.add_callback([this](auto...) {
            this->window.requestRedraw = true;
        });
        [[maybe_unused]] ttlet on_label_cbid = this->onLabel.add_callback([this](auto...) {
            requestConstraint = true;
        });
        [[maybe_unused]] ttlet off_label_cbid = this->offLabel.add_callback([this](auto...) {
            requestConstraint = true;
        });
    }

    ~ToggleWidget() {}

    [[nodiscard]] bool updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (Widget::updateConstraints()) {
            onLabelCell = std::make_unique<TextCell>(*onLabel, theme->labelStyle);
            offLabelCell = std::make_unique<TextCell>(*offLabel, theme->labelStyle);

            ttlet minimumHeight =
                std::max({onLabelCell->preferredExtent().height(), offLabelCell->preferredExtent().height(), Theme::smallSize});

            ttlet minimumWidth = std::max({onLabelCell->preferredExtent().width(), offLabelCell->preferredExtent().width()}) +
                Theme::smallSize * 2.0f + Theme::margin;

            _preferred_size = interval_vec2::make_minimum(minimumWidth, minimumHeight);
            _preferred_base_line = relative_base_line{VerticalAlignment::Top, -Theme::smallSize * 0.5f};

            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        need_layout |= std::exchange(requestLayout, false);
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

        return Widget::updateLayout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        drawToggle(context);
        drawSlider(context);
        drawLabel(context);
        Widget::draw(std::move(context), display_time_point);
    }

    bool handleMouseEvent(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        if (Widget::handleMouseEvent(event)) {
            return true;

        } else if (event.cause.leftButton) {
            if (*enabled) {
                if (event.type == MouseEvent::Type::ButtonUp && _window_rectangle.contains(event.position)) {
                    handleCommand(command::gui_activate);
                }
            }
            return true;

        } else if (parent) {
            return parent->handleMouseEvent(event);
        }
        return false;
    }

    void handleCommand(command command) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        if (!*enabled) {
            return;
        }

        if (command == command::gui_activate) {
            if (compare_then_assign(value, !*value)) {
                window.requestRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    HitBox hitBoxTest(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        if (_window_clipping_rectangle.contains(window_position)) {
            return HitBox{this, _draw_layer, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool acceptsFocus() const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        return *enabled;
    }

private:
    static constexpr hires_utc_clock::duration animationDuration = 150ms;

    aarect toggleRectangle;

    aarect sliderRectangle;
    float sliderMoveRange;

    aarect labelRectangle;

    std::unique_ptr<TextCell> onLabelCell;
    std::unique_ptr<TextCell> offLabelCell;
    std::unique_ptr<TextCell> otherLabelCell;

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
                drawContext.color = hover ? theme->borderColor(_semantic_layer + 1) : theme->borderColor(_semantic_layer);
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
