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

/** A checkbox widget.
 * 
 * @tparam ValueType The type of the value to monitor/modify
 * @tparam TrueValue The value when the checkbox is checked
 * @tparam FaseValue The value when the checkbox is unchecked
 */
template<typename ValueType, ValueType TrueValue, ValueType FalseValue>
class CheckboxWidget final : public Widget {
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

    [[nodiscard]] bool updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (Widget::updateConstraints()) {
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
            ttlet minimumWidth = Theme::smallSize + Theme::margin + minimumWidthOfLabels;

            _preferred_size = interval_vec2::make_minimum(minimumWidth, minimumHeight);
            _preferred_base_line = relative_base_line{VerticalAlignment::Top, -Theme::smallSize * 0.5f};

            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        need_layout |= std::exchange(requestLayout, false);
        if (need_layout) {
            checkboxRectangle = aarect{0.0f, base_line() - Theme::smallSize * 0.5f, Theme::smallSize, Theme::smallSize};

            ttlet labelX = checkboxRectangle.p3().x() + Theme::margin;
            labelRectangle = aarect{labelX, 0.0f, rectangle().width() - labelX, rectangle().height()};

            checkGlyph = to_FontGlyphIDs(ElusiveIcon::Ok);
            ttlet checkGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(checkGlyph);
            checkRectangle = align(checkboxRectangle, scale(checkGlyphBB, Theme::iconSize), Alignment::MiddleCenter);

            minusGlyph = to_FontGlyphIDs(ElusiveIcon::Minus);
            ttlet minusGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(minusGlyph);
            minusRectangle = align(checkboxRectangle, scale(minusGlyphBB, Theme::iconSize), Alignment::MiddleCenter);
        }
        
        return Widget::updateLayout(displayTimePoint, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawCheckBox(context);
        drawCheckMark(context);
        drawLabel(context);
        Widget::draw(std::move(context), display_time_point);
    }

    void handleMouseEvent(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        Widget::handleMouseEvent(event);

        if (*enabled) {
            if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
                ttlet position = fromWindowTransform * event.position;
                if (rectangle().contains(position)) {
                    handleCommand(command::gui_activate);
                }
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
            if (compare_then_assign(value, value == FalseValue ? TrueValue : FalseValue)) {
                window.requestRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        ttlet position = fromWindowTransform * window_position;

        if (rectangle().contains(position)) {
            return HitBox{this, _draw_layer, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool acceptsFocus() const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return *enabled;
    }

private:
    std::unique_ptr<TextCell> trueLabelCell;
    std::unique_ptr<TextCell> falseLabelCell;
    std::unique_ptr<TextCell> otherLabelCell;

    FontGlyphIDs checkGlyph;
    aarect checkRectangle;

    FontGlyphIDs minusGlyph;
    aarect minusRectangle;

    aarect checkboxRectangle;

    aarect labelRectangle;

    void drawCheckBox(DrawContext const &context) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        context.drawBoxIncludeBorder(checkboxRectangle);
    }

    void drawCheckMark(DrawContext context) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        context.transform = mat::T{0.0, 0.0, 0.1f} * context.transform;

        if (*enabled && window.active) {
            context.color = theme->accentColor;
        }

        // Checkmark or tristate.
        if (value == TrueValue) {
            context.drawGlyph(checkGlyph, checkRectangle);
        } else if (value == FalseValue) {
            ;
        } else {
            context.drawGlyph(minusGlyph, minusRectangle);
        }
    }

    void drawLabel(DrawContext context) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (*enabled) {
            context.color = theme->labelStyle.color;
        }

        ttlet &labelCell = value == TrueValue ? trueLabelCell : value == FalseValue ? falseLabelCell : otherLabelCell;

        labelCell->draw(context, labelRectangle, Alignment::TopLeft, base_line(), true);
    }
};

using BooleanCheckboxWidget = CheckboxWidget<bool,true,false>;

} // namespace tt
