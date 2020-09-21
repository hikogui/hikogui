// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../GUI/DrawContext.hpp"
#include "../text/format10.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename ValueType, ValueType ActiveValue>
class RadioButtonWidget : public Widget {
protected:
    aarect radioButtonRectangle;
    aarect pipRectangle;
    aarect labelRectangle;

    std::unique_ptr<TextCell> labelCell;

public:
    observable<ValueType> value;
    observable<std::u8string> label;

    template<typename V, typename... Args>
    RadioButtonWidget(Window &window, Widget *parent, V &&value, l10n const &fmt, Args const &... args) noexcept :
        Widget(window, parent), value(std::forward<V>(value)), label(format(fmt, args...))
    {
        [[maybe_unused]] ttlet value_cbid = value.add_callback([this](auto...) {
            this->window.requestRedraw = true;
        });
        [[maybe_unused]] ttlet label_cbid = label.add_callback([this](auto...) {
            requestConstraint = true;
        });
    }

    template<typename V>
    RadioButtonWidget(Window &window, Widget *parent, V &&value) noexcept :
        RadioButtonWidget(window, parent, std::forward<V>(value), l10n{})
    {
    }

    RadioButtonWidget(Window &window, Widget *parent) noexcept :
        RadioButtonWidget(window, parent, observable<ValueType>{}, l10n{})
    {
    }

    ~RadioButtonWidget() {}

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (ttlet result = Widget::updateConstraints(); result < WidgetUpdateResult::Self) {
            return result;
        }

        labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);

        ttlet minimumHeight = std::max(labelCell->preferredExtent().height(), Theme::smallSize);
        ttlet minimumWidth = Theme::smallSize + Theme::margin + labelCell->preferredExtent().width();

        _size = interval_vec2::make_minimum(minimumWidth, minimumHeight);
        _preferred_base_line = base_line{VerticalAlignment::Top, -Theme::smallSize * 0.5f};
        return WidgetUpdateResult::Self;
    }

    [[nodiscard]] WidgetUpdateResult
    updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (ttlet result = Widget::updateLayout(displayTimePoint, forceLayout); result < WidgetUpdateResult::Self) {
            return result;
        }

        radioButtonRectangle = aarect{0.0f, base_line_position() - Theme::smallSize * 0.5f, Theme::smallSize, Theme::smallSize};

        ttlet labelX = radioButtonRectangle.p3().x() + Theme::margin;
        labelRectangle = aarect{labelX, 0.0f, rectangle().width() - labelX, rectangle().height()};

        pipRectangle = shrink(radioButtonRectangle, 1.5f);
        return WidgetUpdateResult::Self;
    }

    void drawRadioButton(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.cornerShapes = vec{radioButtonRectangle.height() * 0.5f};
        drawContext.drawBoxIncludeBorder(radioButtonRectangle);
    }

    void drawPip(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        // draw pip
        if (value == ActiveValue) {
            if (*enabled && window.active) {
                drawContext.color = theme->accentColor;
            }
            std::swap(drawContext.color, drawContext.fillColor);
            drawContext.cornerShapes = vec{pipRectangle.height() * 0.5f};
            drawContext.drawBoxIncludeBorder(pipRectangle);
        }
    }

    void drawLabel(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        labelCell->draw(drawContext, labelRectangle, Alignment::TopLeft, base_line_position(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawRadioButton(drawContext);
        drawPip(drawContext);
        drawLabel(drawContext);
        Widget::draw(drawContext, displayTimePoint);
    }

    void handleMouseEvent(MouseEvent const &event) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        Widget::handleMouseEvent(event);

        if (*enabled) {
            if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton && rectangle().contains(event.position)) {
                handleCommand(command::gui_activate);
            }
        }
    }

    void handleCommand(command command) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (!*enabled) {
            return;
        }

        if (command == command::gui_activate) {
            if (compare_then_assign(value, ActiveValue)) {
                window.requestRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (rectangle().contains(position)) {
            return HitBox{this, elevation, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool acceptsFocus() const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return *enabled;
    }
};

} // namespace tt
