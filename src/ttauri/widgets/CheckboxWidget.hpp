// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../cells/TextCell.hpp"
#include "../GUI/DrawContext.hpp"
#include "../text/FontBook.hpp"
#include "../text/format10.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename ValueType>
class CheckboxWidget : public Widget {
protected:
    char32_t check = 0x2713;

    std::unique_ptr<TextCell> labelCell;

    FontGlyphIDs checkGlyph;
    aarect checkBoundingBox;

    float button_height;
    float button_width;
    float button_x;
    float button_y;
    float button_middle;
    aarect button_rectangle;

    aarect label_rectangle;

    mat::T label_translate;
    aarect check_rectangle;

    ValueType trueValue;
    ValueType falseValue;

public:
    observable<ValueType> value;
    observable<std::string> label;

    template<typename V>
    CheckboxWidget(Window &window, Widget *parent, V &&value, ValueType trueValue, ValueType falseValue) noexcept :
        Widget(window, parent, {Theme::smallWidth, Theme::smallHeight}),
        trueValue(trueValue),
        falseValue(falseValue),
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

    ~CheckboxWidget() {}

    CheckboxWidget(const CheckboxWidget &) = delete;
    CheckboxWidget &operator=(const CheckboxWidget &) = delete;
    CheckboxWidget(CheckboxWidget&&) = delete;
    CheckboxWidget &operator=(CheckboxWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override {
        Widget::layout(displayTimePoint);

        // The label is located to the right of the toggle.
        ttlet label_x = Theme::smallWidth + Theme::margin;
        label_rectangle = aarect{
            label_x, 0.0f,
            rectangle().width() - label_x, rectangle().height()
        };

        ttlet labelText = *label;
        labelCell = std::make_unique<TextCell>(labelText, theme->labelStyle);
        setFixedHeight(std::max(labelCell->heightForWidth(label_rectangle.width()), Theme::smallHeight));

        button_height = Theme::smallHeight;
        button_width = Theme::smallHeight;
        button_x = Theme::smallWidth - button_width;
        button_y = rectangle().height() - button_height;
        button_rectangle = aarect{button_x, button_y, button_width, button_height};
        button_middle = button_y + button_height * 0.5f;


        ttlet checkFontId = application->fonts->find_font("Arial", FontWeight::Regular, false);
        checkGlyph = application->fonts->find_glyph(checkFontId, Grapheme{check});
        checkBoundingBox = scale(checkGlyph.getBoundingBox(), button_height * 1.2f);

        check_rectangle = align(button_rectangle, checkBoundingBox, Alignment::MiddleCenter);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        // button.
        auto context = drawContext;
        context.drawBoxIncludeBorder(button_rectangle);

        if (*enabled && window.active) {
            context.color = theme->accentColor;
        }

        // Checkmark or tristate.
        if (value == trueValue) {
            context.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
            context.drawGlyph(checkGlyph, check_rectangle);
        } else if (value == falseValue) {
            ;
        } else {
            std::swap(context.color, context.fillColor);
            context.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
            context.drawFilledQuad(shrink(button_rectangle, 3.0f));
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
            if (assign_and_compare(value, value == falseValue ? trueValue : falseValue)) {
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
