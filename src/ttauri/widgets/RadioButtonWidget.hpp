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

template<typename ValueType>
class RadioButtonWidget : public Widget {
protected:
    float button_height;
    float button_width;
    float button_x;
    float button_y;
    float button_middle;
    aarect button_rectangle;
    aarect pip_rectangle;
    aarect label_rectangle;
    mat::T label_translate;
    std::unique_ptr<TextCell> labelCell;

    ValueType activeValue;

public:
    observable<ValueType> value;
    observable<std::string> label;

    template<typename V>
    RadioButtonWidget(Window &window, Widget *parent, V &&value, ValueType activeValue) noexcept :
        Widget(window, parent, vec{Theme::smallWidth, Theme::smallHeight}),
        activeValue(std::move(activeValue)),
        value(std::forward<V>(value)),
        label()
    {
        [[maybe_unused]] ttlet value_cbid = value.add_callback([this](auto...){
            forceRedraw = true;
        });
        [[maybe_unused]] ttlet label_cbid = label.add_callback([this](auto...){
            forceLayout = true;
        });
    }

    ~RadioButtonWidget() {
    }

    RadioButtonWidget(const RadioButtonWidget &) = delete;
    RadioButtonWidget &operator=(const RadioButtonWidget &) = delete;
    RadioButtonWidget(RadioButtonWidget&&) = delete;
    RadioButtonWidget &operator=(RadioButtonWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override {
        Widget::layout(displayTimePoint);

        // The label is located to the right of the toggle.
        ttlet label_x = Theme::smallWidth + Theme::margin;
        label_rectangle = aarect{
            label_x, 0.0f,
            rectangle().width() - label_x, rectangle().height()
        };

        labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);
        setFixedHeight(std::max(labelCell->heightForWidth(label_rectangle.width()), Theme::smallHeight));

        // Prepare coordinates.
        // The button is expanded by half a pixel on each side because it is round.
        button_height = Theme::smallHeight + 1.0f;
        button_width = Theme::smallHeight + 1.0f;
        button_x = (Theme::smallWidth - Theme::smallHeight) - 0.5f;
        button_y = (rectangle().height() - Theme::smallHeight) - 0.5f;
        button_rectangle = aarect{button_x, button_y, button_width, button_height};
        button_middle = button_y + button_height * 0.5f;

        ttlet pip_x = (Theme::smallWidth - Theme::smallHeight) + 1.5f;
        ttlet pip_y = (rectangle().height() - Theme::smallHeight) + 1.5f;
        ttlet pip_width = Theme::smallHeight - 3.0f;
        ttlet pip_height = Theme::smallHeight - 3.0f;
        pip_rectangle = aarect{pip_x, pip_y, pip_width, pip_height};
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override{
        // button.
        auto context = drawContext;
        context.cornerShapes = vec{button_rectangle.height() * 0.5f};
        context.drawBoxIncludeBorder(button_rectangle);

        // draw pip
        if (value == activeValue) {
            if (*enabled && window.active) {
                context.color = theme->accentColor;
            }
            std::swap(context.color, context.fillColor);
            context.cornerShapes = vec{pip_rectangle.height() * 0.5f};
            context.drawBoxIncludeBorder(pip_rectangle);
        }

        labelCell->draw(context, label_rectangle, Alignment::TopLeft, button_middle);
        Widget::draw(drawContext, displayTimePoint);
    }

    void handleMouseEvent(MouseEvent const &event) noexcept override {
        Widget::handleMouseEvent(event);

        if (*enabled) {
            if (
                event.type == MouseEvent::Type::ButtonUp &&
                event.cause.leftButton &&
                rectangle().contains(event.position)
            ) {
                handleCommand("gui.activate"_ltag);
            }
        }
    }

    void handleCommand(string_ltag command) noexcept override {
        if (!*enabled) {
            return;
        }

        if (command == "gui.activate"_ltag) {
            if (assign_and_compare(value, activeValue)) {
                forceRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override {
        if (rectangle().contains(position)) {
            return HitBox{this, elevation, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool acceptsFocus() const noexcept override {
        return *enabled;
    }
};


}
