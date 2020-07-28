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
    std::unique_ptr<TextCell> trueLabelCell;
    std::unique_ptr<TextCell> falseLabelCell;
    std::unique_ptr<TextCell> otherLabelCell;

    FontGlyphIDs checkGlyph;
    aarect checkRectangle;

    FontGlyphIDs minusGlyph;
    aarect minusRectangle;

    aarect checkboxRectangle;

    aarect labelRectangle;

    ValueType trueValue;
    ValueType falseValue;

public:
    observable<ValueType> value;
    observable<std::string> trueLabel;
    observable<std::string> falseLabel;
    observable<std::string> otherLabel;

    CheckboxWidget(Window &window, Widget *parent, ValueType trueValue, ValueType falseValue) noexcept :
        Widget(window, parent, Theme::smallSize, Theme::smallSize),
        trueValue(trueValue),
        falseValue(falseValue)
    {
        [[maybe_unused]] ttlet value_cbid = value.add_callback([this](auto...){
            this->window.requestRedraw = true;
        });
        [[maybe_unused]] ttlet true_label_cbid = trueLabel.add_callback([this](auto...){
            requestLayout = true;
        });
        [[maybe_unused]] ttlet false_label_cbid = falseLabel.add_callback([this](auto...){
            requestLayout = true;
        });
        [[maybe_unused]] ttlet other_label_cbid = otherLabel.add_callback([this](auto...){
            requestLayout = true;
        });
    }

    ~CheckboxWidget() {}

    bool layout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override {
        if (!Widget::layout(displayTimePoint, forceLayout)) {
            return false;
        }

        ttlet lock = std::scoped_lock(mutex);

        checkboxRectangle = aarect{
            0.0f,
            rectangle().height() - Theme::smallSize,
            Theme::smallSize,
            Theme::smallSize
        };

        ttlet labelX = checkboxRectangle.p3().x() + Theme::margin;
        labelRectangle = aarect{
            labelX,
            0.0f,
            rectangle().width() - labelX,
            rectangle().height()
        };

        trueLabelCell = std::make_unique<TextCell>(*trueLabel, theme->labelStyle);
        falseLabelCell = std::make_unique<TextCell>(*falseLabel, theme->labelStyle);
        otherLabelCell = std::make_unique<TextCell>(*otherLabel, theme->labelStyle);

        ttlet preferredHeight = std::max({
            trueLabelCell->preferredExtent().height(),
            falseLabelCell->preferredExtent().height(),
            otherLabelCell->preferredExtent().height(),
            Theme::smallSize
        });

        ttlet preferredWidth = std::max({
            trueLabelCell->preferredExtent().width(),
            falseLabelCell->preferredExtent().width(),
            otherLabelCell->preferredExtent().width(),
        }) + Theme::smallSize + Theme::margin * 2.0f;

        ttlet minimumHeight = std::max({
            trueLabelCell->heightForWidth(labelRectangle.width()),
            falseLabelCell->heightForWidth(labelRectangle.width()),
            otherLabelCell->heightForWidth(labelRectangle.width()),
            Theme::smallSize
        });

        setMaximumWidth(preferredWidth);
        setMaximumHeight(preferredHeight);
        setPreferredWidth(preferredWidth);
        setPreferredHeight(preferredHeight);
        setMinimumHeight(minimumHeight);

        checkGlyph = to_FontGlyphIDs(ElusiveIcon::Ok);
        ttlet checkGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(checkGlyph);
        checkRectangle = align(checkboxRectangle, scale(checkGlyphBB, Theme::iconSize), Alignment::MiddleCenter);

        minusGlyph = to_FontGlyphIDs(ElusiveIcon::Minus);
        ttlet minusGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(minusGlyph);
        minusRectangle = align(checkboxRectangle, scale(minusGlyphBB, Theme::iconSize), Alignment::MiddleCenter);

        return true;
    }

    void drawCheckBox(DrawContext const &drawContext) noexcept {
        drawContext.drawBoxIncludeBorder(checkboxRectangle);
    }

    void drawCheckMark(DrawContext drawContext) noexcept {
        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};

        if (*enabled && window.active) {
            drawContext.color = theme->accentColor;
        }

        // Checkmark or tristate.
        if (value == trueValue) {
            drawContext.drawGlyph(checkGlyph, checkRectangle);
        } else if (value == falseValue) {
            ;
        } else {
            drawContext.drawGlyph(minusGlyph, minusRectangle);
        }
    }

    void drawLabel(DrawContext drawContext) noexcept {
        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        ttlet &labelCell =
            value == trueValue ? trueLabelCell :
            value == falseValue ? falseLabelCell :
            otherLabelCell;

        labelCell->draw(drawContext, labelRectangle, Alignment::TopLeft, center(checkboxRectangle).y(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        drawCheckBox(drawContext);
        drawCheckMark(drawContext);
        drawLabel(drawContext);
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
                handleCommand(command::gui_activate);
            }
        }
    }

    void handleCommand(command command) noexcept override {
        if (!*enabled) {
            return;
        }

        if (command == command::gui_activate) {
            if (assign_and_compare(value, value == falseValue ? trueValue : falseValue)) {
                window.requestRedraw = true;
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
