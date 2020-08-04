// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../cells/TextCell.hpp"
#include "../GUI/DrawContext.hpp"
#include "../observable.hpp"
#include "../text/format10.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename ValueType=bool>
class ToggleWidget : public Widget {
protected:
    static constexpr hires_utc_clock::duration animationDuration = 150ms;

    aarect toggleRectangle;

    aarect sliderRectangle;
    float sliderMoveRange;

    aarect labelRectangle;

    std::unique_ptr<TextCell> onLabelCell;
    std::unique_ptr<TextCell> offLabelCell;
    std::unique_ptr<TextCell> otherLabelCell;

    ValueType onValue;
    ValueType offValue;

public:
    observable<ValueType> value;
    observable<std::string> onLabel;
    observable<std::string> offLabel;
    observable<std::string> otherLabel;

    ToggleWidget(Window &window, Widget *parent, ValueType onValue, ValueType offValue) noexcept :
        Widget(window, parent),
        onValue(onValue),
        offValue(offValue)
    {
        [[maybe_unused]] ttlet value_cbid = this->value.add_callback([this](auto...){
            this->window.requestRedraw = true;
        });
        [[maybe_unused]] ttlet on_label_cbid = this->onLabel.add_callback([this](auto...) {
            requestConstraint = true;
        });
        [[maybe_unused]] ttlet off_label_cbid = this->offLabel.add_callback([this](auto...) {
            requestConstraint = true;
        });
        [[maybe_unused]] ttlet other_label_cbid = this->otherLabel.add_callback([this](auto...) {
            requestConstraint = true;
        });
    }

    ~ToggleWidget() {
    }

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override {
        if (ttlet result = Widget::updateConstraints(); result < WidgetUpdateResult::Self) {
            return result;
        }

        onLabelCell = std::make_unique<TextCell>(*onLabel, theme->labelStyle);
        offLabelCell = std::make_unique<TextCell>(*offLabel, theme->labelStyle);
        otherLabelCell = std::make_unique<TextCell>(*otherLabel, theme->labelStyle);

        ttlet minimumHeight = std::max({
            onLabelCell->preferredExtent().height(),
            offLabelCell->preferredExtent().height(),
            otherLabelCell->preferredExtent().height(),
            Theme::smallSize
            });

        ttlet minimumWidth = std::max({
            onLabelCell->preferredExtent().width(),
            offLabelCell->preferredExtent().width(),
            otherLabelCell->preferredExtent().width(),
            }) + Theme::smallSize * 2.0f + Theme::margin * 2.0f;

        window.stopConstraintSolver();
        window.replaceConstraint(minimumWidthConstraint, width >= minimumWidth);
        window.replaceConstraint(minimumHeightConstraint, height >= minimumHeight);
        window.replaceConstraint(baseConstraint, base == top - Theme::smallSize * 0.5f);
        window.startConstraintSolver();
        return WidgetUpdateResult::Self;
    }

    [[nodiscard]] WidgetUpdateResult updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override {
        if (ttlet result = Widget::updateLayout(displayTimePoint, forceLayout); result < WidgetUpdateResult::Self) {
            return result;
        }

        ttlet lock = std::scoped_lock(mutex);

        toggleRectangle = aarect{
            -0.5f, // Expand horizontally due to rounded shape
            baseHeight() - Theme::smallSize * 0.5f,
            Theme::smallSize * 2.0f + 1.0f, // Expand horizontally due to rounded shape
            Theme::smallSize
        };

        ttlet labelX = Theme::smallSize * 2.0f + Theme::margin;
        labelRectangle = aarect{
            labelX,
            0.0f,
            rectangle().width() - labelX,
            rectangle().height()
        };

        sliderRectangle = shrink(aarect{
            0.0f,
            toggleRectangle.y(),
            toggleRectangle.height(),
            toggleRectangle.height()
        }, 1.5f);
        
        ttlet sliderMoveWidth = Theme::smallSize * 2.0f - (sliderRectangle.x() * 2.0f);
        sliderMoveRange = sliderMoveWidth - sliderRectangle.width();

        return WidgetUpdateResult::Self;
    }
    
    void drawToggle(DrawContext drawContext) noexcept {
        drawContext.cornerShapes = vec{toggleRectangle.height() * 0.5f};
        drawContext.drawBoxIncludeBorder(toggleRectangle);
    }

    void drawSlider(DrawContext drawContext) noexcept {
        // Prepare animation values.
        ttlet animationProgress = value.animation_progress(animationDuration);
        if (animationProgress < 1.0f) {
            window.requestRedraw = true;
        }

        ttlet animatedValue = to_float(value, animationDuration);

        ttlet positionedSliderRectangle = mat::T2(sliderMoveRange * animatedValue, 0.0f) * sliderRectangle;

        if (value == onValue) {
            if (*enabled && window.active) {
                drawContext.color = theme->accentColor;
            }
        } else {
            if (*enabled && window.active) {
                drawContext.color = hover ?
                    theme->borderColor(nestingLevel() + 1) :
                    theme->borderColor(nestingLevel());
            }
        }
        std::swap(drawContext.color, drawContext.fillColor);
        drawContext.cornerShapes = vec{positionedSliderRectangle.height() * 0.5f};
        drawContext.drawBoxIncludeBorder(positionedSliderRectangle);
    }

    void drawLabel(DrawContext drawContext) noexcept {
        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        ttlet &labelCell =
            value == onValue ? onLabelCell :
            value == offValue ? offLabelCell :
            otherLabelCell;

        labelCell->draw(drawContext, labelRectangle, Alignment::TopLeft, baseHeight(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        drawToggle(drawContext);
        drawSlider(drawContext);
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
            if (assign_and_compare(value, value == offValue ? onValue : offValue)) {
                window.requestRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    HitBox hitBoxTest(vec position) const noexcept override {
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
