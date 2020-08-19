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

template<typename ValueType, ValueType TrueValue, ValueType FalseValue>
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

public:
    observable<ValueType> value;
    observable<std::u8string> trueLabel;
    observable<std::u8string> falseLabel;
    observable<std::u8string> otherLabel;

    CheckboxWidget(Window &window, Widget *parent) noexcept :
        Widget(window, parent)
    {
        [[maybe_unused]] ttlet value_cbid = value.add_callback([this](auto...) {
            this->window.requestRedraw = true;
        });
        [[maybe_unused]] ttlet true_label_cbid = trueLabel.add_callback([this](auto...) {
            requestConstraint = true;
        });
        [[maybe_unused]] ttlet false_label_cbid = falseLabel.add_callback([this](auto...) {
            requestConstraint = true;
        });
        [[maybe_unused]] ttlet other_label_cbid = otherLabel.add_callback([this](auto...) {
            requestConstraint = true;
        });
    }

    ~CheckboxWidget() {}

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (ttlet result = Widget::updateConstraints(); result < WidgetUpdateResult::Self) {
            return result;
        }

        trueLabelCell = std::make_unique<TextCell>(*trueLabel, theme->labelStyle);
        falseLabelCell = std::make_unique<TextCell>(*falseLabel, theme->labelStyle);
        otherLabelCell = std::make_unique<TextCell>(*otherLabel, theme->labelStyle);

        ttlet minimumHeight = std::max(
            {trueLabelCell->preferredExtent().height(),
             falseLabelCell->preferredExtent().height(),
             otherLabelCell->preferredExtent().height(),
             Theme::smallSize});

        ttlet minimumWidthOfLabels = std::max(
            {trueLabelCell->preferredExtent().width(),
             falseLabelCell->preferredExtent().width(),
             otherLabelCell->preferredExtent().width()});
        ttlet minimumWidth = minimumWidthOfLabels + Theme::smallSize + Theme::margin * 2.0f;

        window.stopConstraintSolver();
        window.replaceConstraint(minimumWidthConstraint, width >= minimumWidth);
        window.replaceConstraint(minimumHeightConstraint, height >= minimumHeight);
        window.replaceConstraint(baseConstraint, base == top - Theme::smallSize * 0.5f);
        window.startConstraintSolver();
        return WidgetUpdateResult::Self;
    }

    [[nodiscard]] WidgetUpdateResult
    updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (ttlet result = Widget::updateLayout(displayTimePoint, forceLayout); result < WidgetUpdateResult::Self) {
            return result;
        }

        checkboxRectangle = aarect{0.0f, baseHeight() - Theme::smallSize * 0.5f, Theme::smallSize, Theme::smallSize};

        ttlet labelX = checkboxRectangle.p3().x() + Theme::margin;
        labelRectangle = aarect{labelX, 0.0f, rectangle().width() - labelX, rectangle().height()};

        checkGlyph = to_FontGlyphIDs(ElusiveIcon::Ok);
        ttlet checkGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(checkGlyph);
        checkRectangle = align(checkboxRectangle, scale(checkGlyphBB, Theme::iconSize), Alignment::MiddleCenter);

        minusGlyph = to_FontGlyphIDs(ElusiveIcon::Minus);
        ttlet minusGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(minusGlyph);
        minusRectangle = align(checkboxRectangle, scale(minusGlyphBB, Theme::iconSize), Alignment::MiddleCenter);

        return WidgetUpdateResult::Self;
    }

    void drawCheckBox(DrawContext const &drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.drawBoxIncludeBorder(checkboxRectangle);
    }

    void drawCheckMark(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};

        if (*enabled && window.active) {
            drawContext.color = theme->accentColor;
        }

        // Checkmark or tristate.
        if (value == TrueValue) {
            drawContext.drawGlyph(checkGlyph, checkRectangle);
        } else if (value == FalseValue) {
            ;
        } else {
            drawContext.drawGlyph(minusGlyph, minusRectangle);
        }
    }

    void drawLabel(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        ttlet &labelCell = value == TrueValue ? trueLabelCell : value == FalseValue ? falseLabelCell : otherLabelCell;

        labelCell->draw(drawContext, labelRectangle, Alignment::TopLeft, baseHeight(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawCheckBox(drawContext);
        drawCheckMark(drawContext);
        drawLabel(drawContext);
        Widget::draw(drawContext, displayTimePoint);
    }

    void handleMouseEvent(MouseEvent const &event) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        Widget::handleMouseEvent(event);

        if (*enabled) {
            if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton && rectangle().contains(event.position)) {
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
            if (assign_and_compare(value, value == FalseValue ? TrueValue : FalseValue)) {
                window.requestRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (rectangle().contains(position)) {
            return HitBox{this, elevation, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool acceptsFocus() const noexcept override
    {
        return *enabled;
    }
};

using BooleanCheckboxWidget = CheckboxWidget<bool,true,false>;

} // namespace tt
