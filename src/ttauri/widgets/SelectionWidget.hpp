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

    aarect valueRectangle;
    aarect leftBoxRectangle;

    FontGlyphIDs chevronsGlyph;
    aarect chevronsRectangle;

    aarect overlayWindowRectangle;
    aarect overlayRectangle;

    bool selecting = false;
    ValueType choice;
public:
    observable<ValueType> value;
    observable<std::vector<std::pair<ValueType,std::string>>> options;

    SelectionWidget(Window &window, Widget *parent, ValueType defaultValue) noexcept :
        Widget(window, parent, {Theme::smallSize, Theme::smallSize}),
        defaultValue(defaultValue)
    {
        [[maybe_unused]] ttlet value_cbid = value.add_callback([this](auto...){
            forceRedraw = true;
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

        leftBoxRectangle = aarect{
            0.0f, 0.0f,
            Theme::smallSize, rectangle().height()
        };

        ttlet valueX = leftBoxRectangle.p3().x() + Theme::margin;
        ttlet valueWidth = rectangle().width() - valueX - Theme::margin;

        optionCells.clear();
        auto valueHeight = 0.0f;
        for (ttlet &[tag, labelText]: *options) {
            optionCells.emplace_back(tag, std::make_unique<TextCell>(labelText, theme->labelStyle));

            valueHeight = std::max(valueHeight, optionCells.back().second->heightForWidth(valueWidth));
        }

        if (valueHeight != 0.0f) {
            setFixedHeight(valueHeight + Theme::margin * 2.0f);
        } else {
            setFixedHeight(Theme::smallSize);
        }

        // The label is located to the right of the selection box icon.
        valueRectangle = aarect{
            valueX, rectangle().height() - valueHeight - Theme::margin,
            valueWidth, valueHeight
        };

        chevronsGlyph = to_FontGlyphIDs(ElusiveIcon::ChevronUp);
        ttlet chevronsGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(chevronsGlyph);
        chevronsRectangle = align(leftBoxRectangle, scale(chevronsGlyphBB, Theme::iconSize), Alignment::MiddleCenter);

        ttlet maximumOverlayHeight = window.widget->contentExtent().height();

        // Calculate the height of the option, and the location of the current selected option.
        auto optionsHeight = 0.0f;
        auto currentSelectedOptionY = optionsHeight;
        for (ttlet &[tag, optionCell]: optionCells) {
            optionsHeight -= Theme::margin;
            ttlet optionTop = optionsHeight;

            if (tag == *value) {
                // The height of the current selected option is set to the same height
                // as the underlying widget. This way there is a perfect overlap.
                optionsHeight -= valueHeight;

                // Get the location of the middle of the selected option.
                currentSelectedOptionY = (optionsHeight + optionTop) * 0.5f;

            } else {
                optionsHeight -= optionCell->heightForWidth(valueRectangle.width());
            }
        }
        optionsHeight -= Theme::margin;
        optionsHeight = -optionsHeight;

        // Get the coordinate to the selected option, from the bottom of the options-list.
        currentSelectedOptionY = optionsHeight + currentSelectedOptionY;

        // Calculate overlay dimensions and position.
        ttlet windowRectangle_ = windowRectangle();
        ttlet overlayWidth = rectangle().width() - Theme::smallSize;
        ttlet overlayWindowX = windowRectangle().x() + Theme::smallSize;
        ttlet overlayHeight = std::min(optionsHeight, maximumOverlayHeight);
        auto overlayWindowY = (windowRectangle_.y() + windowRectangle_.height() * 0.5f) - currentSelectedOptionY;
            
        // Adjust overlay to fully cover the selection widget.
        //if (overlayWindowY > windowRectangle().y()) {
        //    overlayWindowY = windowRectangle().y();
        //} else if (overlayWindowY + overlayHeight < windowRectangle().y() + windowRectangle().height()) {
        //    overlayWindowY = windowRectangle().y() + windowRectangle().height() - overlayHeight;
        //}

        // Adjust overlay to fit inside the window.
        overlayWindowY = std::clamp(
            overlayWindowY,
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

    void drawOptionHighlight(DrawContext drawContext, ValueType tag, aarect optionRectangle) noexcept {
        
        ttlet optionRectangle_ = aarect{
            optionRectangle.x() - Theme::margin,
            optionRectangle.y() - Theme::margin,
            optionRectangle.width() + Theme::margin * 2.0f,
            optionRectangle.height() + Theme::margin * 2.0f
        };

        if (tag == choice) {
            drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.002f};
            drawContext.fillColor = theme->accentColor;
        } else {
            drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
            drawContext.fillColor = theme->fillColor(nestingLevel());
        }
        drawContext.drawFilledQuad(optionRectangle_);
    }

    void drawOptionLabel(DrawContext drawContext, Cell const &optionCell, aarect optionRectangle) noexcept {
        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.003f};

        drawContext.color = theme->labelStyle.color;
        optionCell.draw(drawContext, optionRectangle, Alignment::MiddleLeft, center(optionRectangle).y(), true);
    }

    void drawOverlayOutline(DrawContext drawContext) noexcept {
        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.010f};
        drawContext.fillColor = drawContext.fillColor.a(0.0f);
        drawContext.drawBoxIncludeBorder(overlayRectangle);
    }

    void drawOverlay(DrawContext const &drawContext) noexcept {
        drawOverlayOutline(drawContext);

        auto y = overlayRectangle.p3().y();
        for (ttlet &[tag, optionCell] : optionCells) {
            y -= Theme::margin;
            ttlet topY = y;

            ttlet optionHeight =
                tag == *value ? valueRectangle.height() :
                optionCell->heightForWidth(valueRectangle.width());

            y -= optionHeight;
            ttlet bottomY = y;

            ttlet optionRectangle = aarect {
                overlayRectangle.x() + Theme::margin,
                bottomY,
                valueRectangle.width(),
                optionHeight
            };

            drawOptionHighlight(drawContext, tag, optionRectangle);
            drawOptionLabel(drawContext, *optionCell, optionRectangle);
        }
    }

    void drawOutline(DrawContext drawContext) noexcept {
        drawContext.cornerShapes = Theme::roundingRadius;
        drawContext.drawBoxIncludeBorder(rectangle());
    }

    void drawLeftBox(DrawContext drawContext) noexcept {
        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
        //if (*enabled && window.active) {
        //    drawContext.color = theme->accentColor;
        //}
        drawContext.fillColor = drawContext.color;
        drawContext.cornerShapes = vec{Theme::roundingRadius, 0.0f, Theme::roundingRadius, 0.0f};
        drawContext.drawBoxIncludeBorder(leftBoxRectangle);
    }

    void drawChevrons(DrawContext drawContext) noexcept {
        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.002f};
        drawContext.color = *enabled ? theme->foregroundColor : drawContext.fillColor;
        drawContext.drawGlyph(chevronsGlyph, chevronsRectangle);
    }

    void drawValue(DrawContext drawContext) noexcept {
        auto i = std::find_if(optionCells.cbegin(), optionCells.cend(), [this](ttlet &item) {
            return item.first == value;
        });

        if (i == optionCells.cend()) {
            return;
        }

        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
        drawContext.color = *enabled ? theme->labelStyle.color : drawContext.color;
        i->second->draw(drawContext, valueRectangle, Alignment::MiddleLeft, center(chevronsRectangle).y(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        drawOutline(drawContext);
        drawLeftBox(drawContext);
        drawChevrons(drawContext);
        drawValue(drawContext);

        if (selecting) {
            auto overlayContext = drawContext;
            overlayContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.250f};
            overlayContext.clippingRectangle = expand(overlayWindowRectangle, Theme::borderWidth * 0.5f);
            drawOverlay(overlayContext);
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

        switch (command) {
        
        case command::gui_up: {
            std::optional<ValueType> prev_tag;
            for (ttlet &[tag, cell] : optionCells) {
                if (tag == choice && prev_tag.has_value()) {
                    choice = *prev_tag;
                    break;
                }
                prev_tag = tag;
            }
            } break;

        case command::gui_down: {
            bool found = false;
            for (ttlet &[tag, cell] : optionCells) {
                if (found) {
                    choice = tag;
                    break;
                }
                if (tag == choice) {
                    found = true;
                }
            }
            }break;

        case command::gui_activate:
            if (selecting) {
                selecting = false;
                value.store(choice);

            } else {
                selecting = true;
                choice = value.load();
            }
            break;

        case command::gui_widget_next:
        case command::gui_widget_prev:
            if (selecting) {
                selecting = false;
                value.store(choice);
            }
            break;

        case command::gui_escape:
            if (selecting) {
                selecting = false;
            }
            break;

        default:;
        }

        forceLayout = true;
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
