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

template<int ActiveValue>
class ToolbarTabButtonWidget : public Widget {
public:
    observable<int> value;
    observable<std::u8string> label;

    template<typename V, typename... Args>
    ToolbarTabButtonWidget(Window &window, Widget *parent, V &&value, l10n const &fmt, Args const &... args) noexcept :
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
    ToolbarTabButtonWidget(Window &window, Widget *parent, V &&value) noexcept :
        ToolbarTabButtonWidget(window, parent, std::forward<V>(value), l10n{})
    {
    }

    ToolbarTabButtonWidget(Window &window, Widget *parent) noexcept :
        ToolbarTabButtonWidget(window, parent, observable<int>{}, l10n{})
    {
    }

    ~ToolbarTabButtonWidget() {}

    [[nodiscard]] bool updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (Widget::updateConstraints()) {
            label_cell = std::make_unique<TextCell>(*label, theme->labelStyle);

            ttlet minimumHeight = label_cell->preferredExtent().height();
            ttlet minimumWidth = label_cell->preferredExtent().width() + 2.0f * Theme::margin;

            _preferred_size = {vec{minimumWidth, minimumHeight}, vec{minimumWidth, std::numeric_limits<float>::infinity()}};
            _preferred_base_line = relative_base_line{VerticalAlignment::Middle, -Theme::margin};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        need_layout |= requestLayout.exchange(false);
        if (need_layout) {
            ttlet offset = Theme::margin + Theme::borderWidth;
            button_rectangle = aarect{
                rectangle().x(), rectangle().y() - offset, rectangle().width(), rectangle().height() + offset
            };
        }

        return Widget::updateLayout(display_time_point, need_layout);
    }

    void drawButton(DrawContext drawContext) noexcept
    {
        if (hover || *value == ActiveValue) {
            drawContext.fillColor = theme->fillColor(nestingLevel() - 2);
            drawContext.color = drawContext.fillColor;
        } else {
            drawContext.fillColor = theme->fillColor(nestingLevel() - 1);
            drawContext.color = drawContext.fillColor;
        }

        if (focus && window.active) {
            drawContext.color = theme->accentColor;
        }

        drawContext.cornerShapes = vec{0.0f, 0.0f, Theme::roundingRadius, Theme::roundingRadius};
        drawContext.drawBoxIncludeBorder(button_rectangle);
    }

    void drawLabel(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.transform = mat::T(0.0f, 0.0f, 0.001f) * drawContext.transform;

        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        label_cell->draw(drawContext, rectangle(), Alignment::MiddleCenter, base_line(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawButton(drawContext);
        drawLabel(drawContext);
        Widget::draw(drawContext, displayTimePoint);
    }

    void handleMouseEvent(MouseEvent const &event) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        Widget::handleMouseEvent(event);

        if (*enabled) {
            if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton && button_rectangle.contains(event.position)) {
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

        if (button_rectangle.contains(position)) {
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

protected:
    aarect button_rectangle;
    std::unique_ptr<TextCell> label_cell;
};

} // namespace tt
