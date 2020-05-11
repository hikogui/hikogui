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

template<typename ValueType, ValueType TrueValue, ValueType FalseValue>
class CheckboxWidget : public Widget {
protected:
    observer<ValueType> value;

    std::string label = "<unknown>";
    char32_t check = 0x2713;

    Text::ShapedText labelShapedText;
    Text::FontGlyphIDs checkGlyph;
    aarect checkBoundingBox;
public:

    CheckboxWidget(Window &window, Widget *parent, observed<ValueType> &value, std::string const label) noexcept :
        Widget(window, parent), value(value, [this](ValueType){ ++this->renderTrigger; }), label(std::move(label))
    {
        if (ssize(label) != 0) {
            window.addConstraint(box.width >= Theme::width);
        } else {
            window.addConstraint(box.width >= Theme::smallWidth);
        }
        window.addConstraint(box.height >= Theme::smallHeight);
    }

    ~CheckboxWidget() {}

    CheckboxWidget(const CheckboxWidget &) = delete;
    CheckboxWidget &operator=(const CheckboxWidget &) = delete;
    CheckboxWidget(CheckboxWidget&&) = delete;
    CheckboxWidget &operator=(CheckboxWidget &&) = delete;

    void draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept override{
        // Prepare coordinates.
        let rectangle = box.currentRectangle();

        let button_height = Theme::smallHeight;
        let button_width = Theme::smallHeight;
        let button_x = Theme::smallWidth - button_width;
        let button_y = (rectangle.height() - button_height) * 0.5f;
        let button_rectangle = aarect{button_x, button_y, button_width, button_height};

        let label_x = Theme::smallWidth + theme->margin;
        let label_y = 0.0f;
        let label_width = rectangle.width() - label_x;
        let label_height = rectangle.height();
        let label_rectangle = aarect{label_x, label_y, label_width, label_height};

        // Prepare labels.
        if (renderTrigger.check(displayTimePoint) >= 2) {
            labelShapedText = Text::ShapedText(label, theme->labelStyle, HorizontalAlignment::Left, label_width);

            let checkFontId = Text::fontBook->find_font("Arial", Text::FontWeight::Regular, false);
            checkGlyph = Text::fontBook->find_glyph(checkFontId, Text::Grapheme{check});
            checkBoundingBox = scale(checkGlyph.getBoundingBox(), button_height * 1.3f);
        }

        let label_translate = mat::align(label_rectangle, aarect{labelShapedText.extent}, Alignment::MiddleLeft);
        let check_rectangle = align(button_rectangle, checkBoundingBox, Alignment::MiddleCenter);

        // button.
        auto context = drawContext;
        context.drawBox(button_rectangle);

        // Checkmark or tristate.
        if (enabled) {
            context.fillColor = theme->accentColor;
            context.color = theme->accentColor;
            if (value == TrueValue) {
                context.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
                context.drawGlyph(checkGlyph, check_rectangle);
            } else if (value == FalseValue) {
                ;
            } else {
                context.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
                context.drawFilledQuad(shrink(button_rectangle, 4.0f));
            }
        } else {
            context.fillColor = theme->borderColor(nestingLevel() - 1);
            context.color = theme->borderColor(nestingLevel() - 1);
            if (value == TrueValue) {
                context.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
                context.drawGlyph(checkGlyph, check_rectangle);
            } else if (value == FalseValue) {
                ;
            } else {
                context.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
                context.drawFilledQuad(shrink(button_rectangle, 4.0f));
            }
        }
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
            if (assign_and_compare(value, value == FalseValue ? TrueValue : FalseValue)) {
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
