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
    
    struct OptionListEntry {
        ValueType tag;
        std::unique_ptr<TextCell> cell;
        aarect cellRectangle;
        aarect backgroundRectangle;

        OptionListEntry(ValueType tag, std::unique_ptr<TextCell> cell) noexcept :
            tag(tag), cell(std::move(cell)), cellRectangle(), backgroundRectangle() {}
    };

    float optionListHeight;
    std::vector<OptionListEntry> optionList;
    float selectedOptionY;

    aarect optionRectangle;
    aarect leftBoxRectangle;

    FontGlyphIDs chevronsGlyph;
    aarect chevronsRectangle;

    aarect overlayWindowRectangle;
    aarect overlayRectangle;

    bool selecting = false;
    ValueType chosenOption;
    std::optional<ValueType> hoverOption;
    std::optional<ValueType> clickedOption;
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

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override {
        ttlet lock = std::scoped_lock(mutex);

        Widget::layout(displayTimePoint);

        leftBoxRectangle = aarect{
            0.0f, 0.0f,
            Theme::smallSize, rectangle().height()
        };

        ttlet optionX = leftBoxRectangle.p3().x() + Theme::margin;
        ttlet optionWidth = rectangle().width() - optionX - Theme::margin;

        // Create a list of cells, one for each option and calculate
        // the optionHeight based on the option which is the tallest.
        optionList.clear();
        auto optionHeight = 0.0f;
        auto preferredWidth = 0.0f;
        auto preferredHeight = 0.0f;
        for (ttlet &[tag, labelText]: *options) {
            auto cell = std::make_unique<TextCell>(labelText, theme->labelStyle);
            optionHeight = std::max(optionHeight, cell->heightForWidth(optionWidth));
            preferredWidth = std::max(preferredWidth, cell->preferredExtent().width());
            preferredHeight = std::max(preferredHeight, cell->preferredExtent().height());
            optionList.emplace_back(tag, std::move(cell));
        }

        // Set the widget height to the tallest option, fallback to a small size widget.
        if (optionHeight == 0.0f) {
            optionHeight = Theme::smallSize;
        }

        setMaximumWidth(preferredWidth + Theme::smallSize + Theme::margin * 2.0f);
        setMaximumHeight(preferredHeight + Theme::margin * 2.0f);
        setPreferredWidth(preferredWidth + Theme::smallSize + Theme::margin * 2.0f);
        setPreferredHeight(preferredHeight + Theme::margin * 2.0f);
        setMinimumHeight(optionHeight + Theme::margin * 2.0f);

        // Calculate the rectangles for each cell in the optionList
        optionListHeight = (optionHeight + Theme::margin * 2.0f) * nonstd::ssize(optionList);
        ttlet optionListWidth = optionWidth + Theme::margin * 2.0f;
        auto y = optionListHeight;
        selectedOptionY = 0.0f;
        for (auto &&option: optionList) {
            y -= Theme::margin;
            y -= optionHeight;
            option.cellRectangle = aarect{Theme::margin, y, optionWidth, optionHeight};
            option.backgroundRectangle = expand(option.cellRectangle, Theme::margin);
            y -= Theme::margin;

            if (option.tag == value) {
                selectedOptionY = y;
            }
        }

        // The window height, excluding the top window decoration.
        ttlet windowHeight = window.widget->extent().height() - Theme::toolbarHeight;

        // Calculate overlay dimensions and position.
        ttlet overlayWidth = optionListWidth;
        ttlet overlayWindowX = windowRectangle().x() + Theme::smallSize;
        ttlet overlayHeight = std::min(optionListHeight, windowHeight);
        auto overlayWindowY = windowRectangle().y() - selectedOptionY;
            
        // Adjust overlay to fit inside the window, below the top window decoration.
        overlayWindowY = std::clamp(
            overlayWindowY,
            0.0f,
            windowHeight - overlayHeight
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
            overlayWindowX - windowRectangle().x(),
            overlayWindowY - windowRectangle().y(),
            overlayWidth,
            overlayHeight
        };

        // The label is located to the right of the selection box icon.
        optionRectangle = aarect{
            optionX, rectangle().height() - optionHeight - Theme::margin,
            optionWidth, optionHeight
        };

        chevronsGlyph = to_FontGlyphIDs(ElusiveIcon::ChevronUp);
        ttlet chevronsGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(chevronsGlyph);
        chevronsRectangle = align(leftBoxRectangle, scale(chevronsGlyphBB, Theme::iconSize), Alignment::MiddleCenter);
    }

    void drawOptionHighlight(DrawContext drawContext, OptionListEntry const &option) noexcept {
        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};

        if (option.tag == chosenOption && !clickedOption.has_value()) {
            drawContext.fillColor = theme->accentColor;
        } else if (option.tag == clickedOption) {
            drawContext.fillColor = theme->accentColor;
        } else if (option.tag == hoverOption) {
            drawContext.fillColor = theme->fillColor(nestingLevel() + 1);
        } else {
            drawContext.fillColor = theme->fillColor(nestingLevel());
        }
        drawContext.drawFilledQuad(option.backgroundRectangle);
    }

    void drawOptionLabel(DrawContext drawContext, OptionListEntry const &option) noexcept {
        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.003f};
        drawContext.color = theme->labelStyle.color;
        option.cell->draw(drawContext, option.cellRectangle, Alignment::MiddleLeft, center(option.cellRectangle).y(), true);
    }

    void drawOption(DrawContext drawContext, OptionListEntry const &option) noexcept {
        drawOptionHighlight(drawContext, option);
        drawOptionLabel(drawContext, option);
    }

    void drawOverlayOutline(DrawContext drawContext) noexcept {
        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.010f};
        drawContext.fillColor = drawContext.fillColor.a(0.0f);
        drawContext.drawBoxIncludeBorder(overlayRectangle);
    }

    void drawOverlay(DrawContext const &drawContext) noexcept {
        drawOverlayOutline(drawContext);

        auto optionListContext = drawContext;
        optionListContext.transform = mat::T{overlayRectangle.x(), overlayRectangle.y()} * drawContext.transform;
        for (ttlet &option : optionList) {
            drawOption(optionListContext, option);
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
        auto i = std::find_if(optionList.cbegin(), optionList.cend(), [this](ttlet &item) {
            return item.tag == value;
        });

        if (i == optionList.cend()) {
            return;
        }

        drawContext.transform = drawContext.transform * mat::T{0.0, 0.0, 0.001f};
        drawContext.color = *enabled ? theme->labelStyle.color : drawContext.color;
        i->cell->draw(drawContext, optionRectangle, Alignment::MiddleLeft, center(chevronsRectangle).y(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        ttlet lock = std::scoped_lock(mutex);

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

    void handleKeyboardEvent(KeyboardEvent const &event) noexcept override {
        ttlet lock = std::scoped_lock(mutex);
        Widget::handleKeyboardEvent(event);

        if (event.type == KeyboardEvent::Type::Exited) {
            handleCommand(command::gui_escape);
        }
    }

    void handleMouseEvent(MouseEvent const &event) noexcept override {
        ttlet lock = std::scoped_lock(mutex);
        Widget::handleMouseEvent(event);

        if (*enabled) {
            if (selecting) {
                auto mouseInListPosition = mat::T{-overlayRectangle.x(), -overlayRectangle.y()} * event.position;

                if (overlayRectangle.contains(event.position)) {
                    for (ttlet &option : optionList) {
                        if (option.backgroundRectangle.contains(mouseInListPosition)) {
                            if (hoverOption != option.tag) {
                                forceRedraw = true;
                                hoverOption = option.tag;
                            }
                        }
                    }

                } else {
                    if (hoverOption.has_value()) {
                        forceRedraw = true;
                        hoverOption = {};
                    }
                }

                if (event.type == MouseEvent::Type::ButtonDown && event.cause.leftButton) {
                    clickedOption = hoverOption;
                    forceRedraw = true;
                }
                if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
                    if (clickedOption.has_value() && clickedOption == hoverOption) {
                        chosenOption = *clickedOption;
                        handleCommand(command::gui_activate);
                    }
                    clickedOption = {};
                    forceRedraw = true;
                }

            } else {
                if (
                    event.type == MouseEvent::Type::ButtonUp &&
                    event.cause.leftButton &&
                    rectangle().contains(event.position)
                ) {
                    handleCommand(command::gui_activate);
                }
            }
        }
    }

    void handleCommand(command command) noexcept override {
        ttlet lock = std::scoped_lock(mutex);

        if (!*enabled) {
            return;
        }

        switch (command) {
        case command::gui_up: {
            std::optional<ValueType> prev_tag;
            for (ttlet &option : optionList) {
                if (option.tag == chosenOption && prev_tag.has_value()) {
                    chosenOption = *prev_tag;
                    break;
                }
                prev_tag = option.tag;
            }
            } break;

        case command::gui_down: {
            bool found = false;
            for (ttlet &option : optionList) {
                if (found) {
                    chosenOption = option.tag;
                    break;
                } else if (option.tag == chosenOption) {
                    found = true;
                }
            }
            } break;

        case command::gui_activate:
            if (selecting) {
                selecting = false;
                value.store(chosenOption);

            } else {
                selecting = true;
                chosenOption = value.load();
            }
            break;

        case command::gui_widget_next:
        case command::gui_widget_prev:
            if (selecting) {
                selecting = false;
                value.store(chosenOption);
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
        ttlet lock = std::scoped_lock(mutex);

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
