// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
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
public:

    RadioButtonWidget(Window &window, Widget *parent, observed<ValueType> &value, std::string const label) noexcept :
        Widget(window, parent), value(value, [this](ValueType){ ++this->renderTrigger; }), label(std::move(label))
    {
        if (ssize(label) != 0) {
            window.addConstraint(box.width >= Theme::width);
        } else {
            window.addConstraint(box.width >= Theme::smallWidth);
        }
        window.addConstraint(box.height >= Theme::smallHeight);
    }

    ~RadioButtonWidget() {}

    RadioButtonWidget(const RadioButtonWidget &) = delete;
    RadioButtonWidget &operator=(const RadioButtonWidget &) = delete;
    RadioButtonWidget(RadioButtonWidget&&) = delete;
    RadioButtonWidget &operator=(RadioButtonWidget &&) = delete;

    void draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept override{
        // Prepare coordinates.
        let rectangle = box.currentRectangle();

        let button_height = Theme::smallHeight;
        let button_width = Theme::smallHeight;
        let button_x = Theme::smallWidth - button_width;
        let button_y = (rectangle.height() - button_height) * 0.5f;
        // Radio button should be slightly larger due to its round shape.
        let button_rectangle = expand(rect{button_x, button_y, button_width, button_height}, 0.5f);

        let pip_rectangle = shrink(button_rectangle, Theme::borderWidth + 1.0f);

        let label_x = Theme::smallWidth + theme->margin;
        let label_y = 0.0f;
        let label_width = rectangle.width() - label_x;
        let label_height = rectangle.height();
        let label_rectangle = rect{label_x, label_y, label_width, label_height};

        // Prepare labels.
        if (renderTrigger.check(displayTimePoint) >= 2) {
            labelShapedText = Text::ShapedText(label, theme->labelStyle, HorizontalAlignment::Left, label_width);
            window.device->SDFPipeline->prepareAtlas(labelShapedText);
        }
        let label_translate = mat::T{label_rectangle.align(labelShapedText.extent, Alignment::MiddleLeft)};

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
        context.transform = drawContext.transform * label_translate * mat::T{0.0, 0.0, 0.001f};
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
