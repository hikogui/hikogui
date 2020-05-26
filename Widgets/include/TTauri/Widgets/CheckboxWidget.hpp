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

template<typename ValueType, ValueType TrueValue, ValueType FalseValue>
class CheckboxWidget : public Widget {
protected:
    observer<ValueType> value;

    std::string label = "<unknown>";
    char32_t check = 0x2713;

    Text::ShapedText labelShapedText;
    Text::FontGlyphIDs checkGlyph;
    aarect checkBoundingBox;

    float button_height;
    float button_width;
    float button_x;
    float button_y;
    aarect button_rectangle;

    aarect label_rectangle;

    mat::T label_translate;
    aarect check_rectangle;
public:

    CheckboxWidget(Window &window, Widget *parent, observable<ValueType> &value, std::string const label) noexcept :
        Widget(window, parent, vec{ssize(label) == 0 ? Theme::smallWidth : Theme::width, Theme::smallHeight}),
        value(value, [this](auto...){ forceRedraw = true; }),
        label(std::move(label))
    {}

    ~CheckboxWidget() {}

    CheckboxWidget(const CheckboxWidget &) = delete;
    CheckboxWidget &operator=(const CheckboxWidget &) = delete;
    CheckboxWidget(CheckboxWidget&&) = delete;
    CheckboxWidget &operator=(CheckboxWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override {
        Widget::layout(displayTimePoint);

        // The label is located to the right of the toggle.
        let label_x = Theme::smallWidth + Theme::margin;
        label_rectangle = aarect{
            label_x, 0.0f,
            rectangle.width() - label_x, rectangle.height()
        };

        labelShapedText = Text::ShapedText(label, theme->labelStyle, label_rectangle.width(), Alignment::TopLeft);
        label_translate = labelShapedText.T(label_rectangle);
        setFixedHeight(std::max(labelShapedText.boundingBox.height(), Theme::smallHeight));

        button_height = Theme::smallHeight;
        button_width = Theme::smallHeight;
        button_x = Theme::smallWidth - button_width;
        button_y = rectangle.height() - button_height;
        button_rectangle = aarect{button_x, button_y, button_width, button_height};

        check_rectangle = align(button_rectangle, checkBoundingBox, Alignment::MiddleCenter);

        let checkFontId = Text::fontBook->find_font("Arial", Text::FontWeight::Regular, false);
        checkGlyph = Text::fontBook->find_glyph(checkFontId, Text::Grapheme{check});
        checkBoundingBox = scale(checkGlyph.getBoundingBox(), button_height * 1.2f);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        // button.
        auto context = drawContext;
        context.drawBoxIncludeBorder(button_rectangle);

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
                context.drawFilledQuad(shrink(button_rectangle, 3.0f));
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
                context.drawFilledQuad(shrink(button_rectangle, 3.0f));
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
            if (
                event.type == GUI::MouseEvent::Type::ButtonUp &&
                event.cause.leftButton &&
                rectangle.contains(event.position)
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
            if (assign_and_compare(value, value == FalseValue ? TrueValue : FalseValue)) {
                forceRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override {
        if (rectangle.contains(position)) {
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
