// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../cells/TextCell.hpp"
#include "../GUI/DrawContext.hpp"
#include "../text/FontBook.hpp"
#include "../text/format10.hpp"
#include "../text/ElusiveIcons.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename ValueType>
class SelectionWidget : public Widget {
protected:
    ValueType defaultValue;

    std::vector<std::pair<ValueType,std::unique_ptr<TextCell>>> optionCells;

    aarect leftSideRectangle;
    aarect valueRectangle;

    FontGlyphIDs selectionIconGlyph;
    aarect selectionIconRectangle;
    float selectionIconMiddle;
    aarect selectionIconGlyphRectangle;

    aarect overlayWindowRectangle;
    aarect overlayRectangle;

    bool selecting = false;
public:
    observable<ValueType> value;
    observable<std::vector<std::pair<ValueType,std::string>>> options;

    SelectionWidget(Window &window, Widget *parent, ValueType defaultValue) noexcept :
        Widget(window, parent, {Theme::smallWidth, Theme::smallHeight}),
        defaultValue(defaultValue),
        value(),
        options()
    {
        [[maybe_unused]] ttlet value_cbid = value.add_callback([this](auto...){
            forceLayout = true;
        });

        [[maybe_unused]] ttlet options_cbid = options.add_callback([this](auto...){
            forceLayout = true;
        });
    }

    ~SelectionWidget() {}

    SelectionWidget(const SelectionWidget &) = delete;
    SelectionWidget &operator=(const SelectionWidget &) = delete;
    SelectionWidget(SelectionWidget&&) = delete;
    SelectionWidget &operator=(SelectionWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override {
        Widget::layout(displayTimePoint);

        leftSideRectangle = aarect{
            0.0f, 0.0f,
            Theme::smallWidth, rectangle().height()
        };

        optionCells.clear();
        for (ttlet &[tag, labelText]: *options) {
            optionCells.emplace_back(tag, std::make_unique<TextCell>(labelText, theme->labelStyle));
        }

        // Find the selected option and determine the height of the widget.
        ttlet valueX = Theme::smallWidth + Theme::margin;
        ttlet valueWidth = rectangle().width() - valueX;

        auto i = std::find_if(optionCells.cbegin(), optionCells.cend(), [this](ttlet &item) {
            return item.first == value;
        });

        ttlet valueHeight = i != optionCells.cend() ? i->second->heightForWidth(valueWidth) : Theme::smallHeight;
        setFixedHeight(valueHeight + Theme::margin * 2.0f);

        // The label is located to the right of the selection box icon.
        valueRectangle = aarect{
            valueX, rectangle().height() - valueHeight - Theme::margin,
            valueWidth, valueHeight
        };

        // The selection icon rectangle is aligned in the middle
        selectionIconRectangle = aarect{
            Theme::smallWidth * 0.5f, rectangle().height() * 0.5f - Theme::smallHeight * 0.5f,
            Theme::smallWidth * 0.5f, Theme::smallHeight
        };
        selectionIconMiddle = selectionIconRectangle.y() + Theme::smallHeight * 0.5f;

        selectionIconGlyph = to_FontGlyphIDs(ElusiveIcon::ChevronUp);
        ttlet selectionIconGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(selectionIconGlyph);
        selectionIconGlyphRectangle = align(selectionIconRectangle, scale(selectionIconGlyphBB, Theme::iconSize), Alignment::MiddleCenter);

        ttlet maximumOverlayHeight = window.widget->contentExtent().height();

        // Calculate the height of the option, and the location of the current selected option.
        auto optionsHeight = 0.0f;
        auto currentSelectedOptionY = optionsHeight;
        for (ttlet &[tag, optionCell]: optionCells) {
            optionsHeight -= Theme::margin;
            optionsHeight -= optionCell->heightForWidth(valueRectangle.width());
            if (tag == *value) {
                currentSelectedOptionY = optionsHeight;
            }
        }
        optionsHeight -= Theme::margin;
        optionsHeight = -optionsHeight;

        // Get the coordinate to the selected option, from the bottom of the options-list.
        currentSelectedOptionY = optionsHeight + currentSelectedOptionY;

        // Calculate overlay dimensions and position.
        ttlet windowRectangle_ = windowRectangle();
        ttlet overlayWidth = rectangle().width() - Theme::smallWidth * 0.5f;
        ttlet overlayWindowX = windowRectangle().x() + Theme::smallWidth * 0.5f;
        ttlet overlayHeight = std::min(optionsHeight, maximumOverlayHeight);
        ttlet overlayWindowY = std::clamp(
            (windowRectangle_.y() + Theme::margin) - currentSelectedOptionY,
            0.0f,
            maximumOverlayHeight - overlayHeight
        );

        overlayWindowRectangle = aarect{
            overlayWindowX,
            overlayWindowY,
            overlayWidth,
            overlayHeight
        };

        // The overlayRectangle are in the coordinate system of the current widget, so it will
        // extent beyond the current widget.
        overlayRectangle = aarect{
            overlayWindowX - windowRectangle_.x(),
            overlayWindowY - windowRectangle_.y(),
            overlayWidth,
            overlayHeight
        };
    }

    void drawOverlay(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept {
        auto context = drawContext;
        context.drawBoxIncludeBorder(overlayRectangle);

        // Draw all the option labels.
        context.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};

        auto y = overlayRectangle.p3().y();
        for (ttlet &[tag, optionCell] : optionCells) {
            y -= Theme::margin;
            ttlet topY = y;

            ttlet optionHeight = optionCell->heightForWidth(valueRectangle.width());

            y -= optionHeight;
            ttlet bottomY = y;

            ttlet checkboxRectangle = aarect{
                overlayRectangle.x(),
                bottomY + optionHeight * 0.5f - Theme::smallHeight * 0.5f,
                Theme::smallWidth * 0.5f,
                Theme::smallHeight
            };
            ttlet checkboxMiddle = checkboxRectangle.y() + Theme::smallHeight * 0.5f;

            ttlet optionRectangle = aarect {
                overlayRectangle.x() + Theme::smallWidth * 0.5f + Theme::margin,
                bottomY,
                valueRectangle.width(),
                optionHeight
            };

            context.color = theme->labelStyle.color;
            optionCell->draw(context, optionRectangle, Alignment::MiddleLeft, checkboxMiddle, true);
        }
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        auto context = drawContext;

        // Draw the outline
        context.cornerShapes = Theme::roundingRadius;
        context.drawBoxIncludeBorder(rectangle());
        
        // Fill the left side selection box in the accent color.
        context.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
        if (*enabled && window.active) {
            context.color = theme->accentColor;
        }
        context.fillColor = context.color;
        context.cornerShapes = vec{Theme::roundingRadius, 0.0f, Theme::roundingRadius, 0.0f};
        context.drawBoxIncludeBorder(leftSideRectangle);

        // Draw the primary selection box icon.
        context.transform = drawContext.transform * mat::T{0.0, 0.0, 0.002f};
        context.color = *enabled ? theme->foregroundColor : drawContext.fillColor;
        context.drawGlyph(selectionIconGlyph, selectionIconGlyphRectangle);
                
        // Find the selected option and draw the text.
        auto i = std::find_if(optionCells.cbegin(), optionCells.cend(), [this](ttlet &item) {
            return item.first == value;
        });
        if (i != optionCells.cend()) {
            context.color = *enabled ? theme->labelStyle.color : drawContext.color;
            context.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
            i->second->draw(context, valueRectangle, Alignment::MiddleLeft, selectionIconMiddle, true);
        }

        if (selecting) {
            auto overlayContext = drawContext;
            overlayContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.250f};
            overlayContext.clippingRectangle = expand(overlayWindowRectangle, Theme::borderWidth * 0.5f);
            drawOverlay(overlayContext, displayTimePoint);
        }

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
            if (assign_and_compare(selecting, !selecting)) {
                forceRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override {
        if (selecting && overlayRectangle.contains(position)) {
            return HitBox{this, elevation + 25.0f, *enabled ? HitBox::Type::Button : HitBox::Type::Default};

        } else if (rectangle().contains(position)) {
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
