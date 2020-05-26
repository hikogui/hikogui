// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
#include "TTauri/GUI/DrawContext.hpp"
#include "TTauri/Foundation/observer.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri::GUI::Widgets {

template<typename ValueType, ValueType ActiveValue>
class RadioButtonWidget : public Widget {
protected:
    observer<ValueType> value;

    std::string label = "<unknown>";

    Text::ShapedText labelShapedText;

    float button_height;
    float button_width;
    float button_x;
    float button_y;
    aarect button_rectangle;
    aarect pip_rectangle;
    aarect label_rectangle;
    mat::T label_translate;
public:

    template<typename Value>
    RadioButtonWidget(Window &window, Widget *parent, Value &&value, std::string const label) noexcept :
        Widget(window, parent, vec{ssize(label) == 0 ? Theme::smallWidth : Theme::width, Theme::smallHeight}),
        value(std::forward<Value>(value), [this](auto...){ forceRedraw = true; }),
        label(std::move(label))
    {
    }

    ~RadioButtonWidget() {}

    RadioButtonWidget(const RadioButtonWidget &) = delete;
    RadioButtonWidget &operator=(const RadioButtonWidget &) = delete;
    RadioButtonWidget(RadioButtonWidget&&) = delete;
    RadioButtonWidget &operator=(RadioButtonWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override {
        Widget::layout(displayTimePoint);

        // The label is located to the right of the toggle.
        let label_x = Theme::smallWidth + Theme::margin;
        label_rectangle = aarect{
            label_x, 0.0f,
            rectangle().width() - label_x, rectangle().height()
        };

        labelShapedText = Text::ShapedText(label, theme->labelStyle, label_rectangle.width(), Alignment::TopLeft);
        label_translate = labelShapedText.T(label_rectangle);
        setFixedHeight(std::max(labelShapedText.boundingBox.height(), Theme::smallHeight));

        // Prepare coordinates.
        // The button is expanded by half a pixel on each side because it is round.
        button_height = Theme::smallHeight + 1.0f;
        button_width = Theme::smallHeight + 1.0f;
        button_x = (Theme::smallWidth - Theme::smallHeight) - 0.5f;
        button_y = (rectangle().height() - Theme::smallHeight) - 0.5f;
        button_rectangle = aarect{button_x, button_y, button_width, button_height};

        let pip_x = (Theme::smallWidth - Theme::smallHeight) + 1.5f;
        let pip_y = (rectangle().height() - Theme::smallHeight) + 1.5f;
        let pip_width = Theme::smallHeight - 3.0f;
        let pip_height = Theme::smallHeight - 3.0f;
        pip_rectangle = aarect{pip_x, pip_y, pip_width, pip_height};
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override{
        // button.
        auto context = drawContext;
        context.cornerShapes = vec{button_rectangle.height() * 0.5f};
        context.drawBoxIncludeBorder(button_rectangle);

        // draw pip
        context.color = context.fillColor;
        if (value == ActiveValue) {
            if (enabled) {
                context.fillColor = theme->accentColor;
            } else {
                context.fillColor = context.color;
            }
        }
        context.cornerShapes = vec{pip_rectangle.height() * 0.5f};
        context.drawBoxIncludeBorder(pip_rectangle);

        // user defined label.
        context.transform = drawContext.transform * (mat::T{0.0, 0.0, 0.001f} * label_translate);
        context.drawText(labelShapedText);

        Widget::draw(drawContext, displayTimePoint);
    }

    void handleMouseEvent(GUI::MouseEvent const &event) noexcept override {
        Widget::handleMouseEvent(event);

        if (enabled) {
            if (
                event.type == GUI::MouseEvent::Type::ButtonUp &&
                event.cause.leftButton &&
                rectangle().contains(event.position)
            ) {
                handleCommand("gui.activate"_ltag);
            }
        }
    }

    void handleCommand(string_ltag command) noexcept override {
        if (!enabled) {
            return;
        }

        if (command == "gui.activate"_ltag) {
            if (assign_and_compare(value, ActiveValue)) {
                forceRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override {
        if (rectangle().contains(position)) {
            return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool acceptsFocus() const noexcept override {
        return enabled;
    }
};


}
