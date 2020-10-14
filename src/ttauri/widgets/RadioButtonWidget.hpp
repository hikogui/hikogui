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
class RadioButtonWidget final : public Widget {
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

    [[nodiscard]] bool updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (Widget::updateConstraints()) {
            labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);

            ttlet minimumHeight = std::max(labelCell->preferredExtent().height(), Theme::smallSize);
            ttlet minimumWidth = Theme::smallSize + Theme::margin + labelCell->preferredExtent().width();

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
            radioButtonRectangle = aarect{0.0f, base_line() - Theme::smallSize * 0.5f, Theme::smallSize, Theme::smallSize};

            ttlet labelX = radioButtonRectangle.p3().x() + Theme::margin;
            labelRectangle = aarect{labelX, 0.0f, rectangle().width() - labelX, rectangle().height()};

            pipRectangle = shrink(radioButtonRectangle, 1.5f);
        }
        return Widget::updateLayout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawRadioButton(context);
        drawPip(context);
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
            if (compare_then_assign(value, ActiveValue)) {
                window.requestRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override
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
    aarect radioButtonRectangle;
    aarect pipRectangle;
    aarect labelRectangle;

    std::unique_ptr<TextCell> labelCell;

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

        labelCell->draw(drawContext, labelRectangle, Alignment::TopLeft, base_line(), true);
    }
};

} // namespace tt
