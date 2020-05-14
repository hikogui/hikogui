// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/ControlWidget.hpp"
#include "TTauri/GUI/DrawContext.hpp"
#include "TTauri/Foundation/observer.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri::GUI::Widgets {

template<typename ValueType, ValueType ActiveValue>
class RadioButtonWidget : public ControlWidget {
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
    float label_x;
    float label_y;
    float label_width;
    float label_height;
    aarect label_rectangle;
    mat::T label_translate;
public:

    RadioButtonWidget(Window &window, Widget *parent, observed<ValueType> &value, std::string const label) noexcept :
        ControlWidget(window, parent, vec{ssize(label) == 0 ? Theme::smallWidth : Theme::width, Theme::smallHeight}),
        value(value, [this](ValueType){ ++this->renderTrigger; }),
        label(std::move(label))
    {
    }

    ~RadioButtonWidget() {}

    RadioButtonWidget(const RadioButtonWidget &) = delete;
    RadioButtonWidget &operator=(const RadioButtonWidget &) = delete;
    RadioButtonWidget(RadioButtonWidget&&) = delete;
    RadioButtonWidget &operator=(RadioButtonWidget &&) = delete;

    bool needsLayout() const noexcept override {
        return Widget::needsLayout();
    }

    bool layout() noexcept override {
        auto changed = Widget::layout();

        // Prepare coordinates.
        button_height = Theme::smallHeight;
        button_width = Theme::smallHeight;
        button_x = Theme::smallWidth - button_width;
        button_y = (rectangle.height() - button_height) * 0.5f;
        
        // Radio button should be slightly larger due to its round shape.
        button_rectangle = expand(aarect{button_x, button_y, button_width, button_height}, 0.5f);

        pip_rectangle = shrink(button_rectangle, Theme::borderWidth + 1.0f);

        label_x = Theme::smallWidth + theme->margin;
        label_y = 0.0f;
        label_width = rectangle.width() - label_x;
        label_height = rectangle.height();
        label_rectangle = aarect{label_x, label_y, label_width, label_height};

        // Prepare labels.
        labelShapedText = Text::ShapedText(label, theme->labelStyle, Alignment::MiddleLeft, label_width);
        label_translate = labelShapedText.T(label_rectangle);

        return changed;
    }

    void draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept override{
        

        // button.
        auto context = drawContext;
        context.cornerShapes = vec{button_rectangle.height() / 2.0f};
        context.drawBox(button_rectangle);

        // draw pip
        context.color = context.fillColor;
        if (value == ActiveValue) {
            if (enabled) {
                context.fillColor = theme->accentColor;
            } else {
                context.fillColor = context.color;
            }
        }
        context.cornerShapes = vec{pip_rectangle.height() / 2.0f};
        context.drawBox(pip_rectangle);

        // user defined label.
        context.transform = drawContext.transform * (mat::T{0.0, 0.0, 0.001f} * label_translate);
        context.drawText(labelShapedText);

        Widget::draw(drawContext, displayTimePoint);
    }

    void handleMouseEvent(GUI::MouseEvent const &event) noexcept override {
        Widget::handleMouseEvent(event);

        if (enabled) {
            if (event.type == GUI::MouseEvent::Type::ButtonUp && event.cause.leftButton) {
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
                ++renderTrigger;
            }
        }
        Widget::handleCommand(command);
    }

    [[nodiscard]] HitBox hitBoxTest(vec position) noexcept{
        if (box.contains(position)) {
            return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool acceptsFocus() noexcept override {
        return enabled;
    }
};


}
