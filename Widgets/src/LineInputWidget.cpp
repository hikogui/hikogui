// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "TTauri/Widgets/LineInputWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace TTauri::Text;
using namespace std::literals;

LineInputWidget::LineInputWidget(Window &window, Widget *parent, std::string const label) noexcept :
    Widget(window, parent),
    label(std::move(label)),
    field(theme->labelStyle),
    shapedText()
{
}

void LineInputWidget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    auto context = drawContext;
    context.cornerShapes = theme->lineInputCornerShapes;
    context.lineWidth = theme->lineInputBorderWidth;


    if (focus) {
        context.color = theme->accentColor;
        context.fillColor = theme->fillColor(nestingLevel() + 1);
    } else if (hover) {
        context.color = theme->accentColor;
        context.fillColor = theme->fillColor(nestingLevel() + 1);
    } else {
        context.color = theme->fillColor(nestingLevel() + 1);
        context.fillColor = theme->fillColor(nestingLevel());
    }

    // Place the border of the input field rectangle in the middle of a pixel.
    let inputFieldRectangle = shrink(rect{vec{}, box.currentExtent()}, 0.5);
    context.drawBox(inputFieldRectangle);

    let textRectangle = shrink(inputFieldRectangle, 2.0);

    if (renderTrigger.check(displayTimePoint) >= 2) {
        field.setStyleOfAll(theme->labelStyle);

        field.setExtent(textRectangle.extent());
        leftToRightCaret = field.leftToRightCaret();
        partialGraphemeCaret = field.partialGraphemeCaret();
        selectionRectangles = field.selectionRectangles();

        if (ssize(field) == 0) {
            shapedText = ShapedText(label, theme->placeholderLabelStyle, HorizontalAlignment::Left, textRectangle.width());

        } else {
            shapedText = field.shapedText();
        }

        window.device->SDFPipeline->prepareAtlas(shapedText);

        lastUpdateTimePoint = displayTimePoint;
    }

    auto textOffset = textRectangle.align(shapedText.extent, Alignment::MiddleLeft);
    context.transform = context.transform * mat::T(textOffset.z(0.0001f));

    for (let selectionRectangle: selectionRectangles) {
        context.fillColor = theme->textSelectColor;
        context.drawFilledQuad(selectionRectangle);
    }

    if (partialGraphemeCaret) {
        context.fillColor = theme->incompleteGlyphColor;
        context.drawFilledQuad(partialGraphemeCaret);
    }

    // Display the caret and handle blinking.
    auto durationSinceLastUpdate = displayTimePoint - lastUpdateTimePoint;
    auto nrHalfBlinks = static_cast<int64_t>(durationSinceLastUpdate / 500ms);

    auto nextHalfBlinkTime = lastUpdateTimePoint + ((nrHalfBlinks + 1) * 500ms);
    renderTrigger += nextHalfBlinkTime;

    auto blinkIsOn = nrHalfBlinks % 2 == 0;
    if (leftToRightCaret && blinkIsOn && focus && window.active) {
        context.fillColor = theme->cursorColor;
        context.drawFilledQuad(leftToRightCaret);
    }

    context.transform = context.transform * mat::T(0.0f, 0.0f, 0.001f);
    context.drawText(shapedText);

    Widget::draw(drawContext, displayTimePoint);
}


void LineInputWidget::handleCommand(string_ltag command) noexcept
{
    LOG_DEBUG("LineInputWidget: Received command: {}", tt5_decode(command));
    if (!enabled) {
        return;
    }

    // This lock is held during rendering, only update the field when holding this lock.
    auto lock = std::scoped_lock(guiMutex);

    if (command == "text.edit.paste"_ltag) {
        field.handlePaste(window.getTextFromClipboard());
    } else if (command == "text.edit.copy"_ltag) {
        window.setTextOnClipboard(field.handleCopy());
    } else if (command == "text.edit.cut"_ltag) {
        window.setTextOnClipboard(field.handleCut());
    } else {
        field.handleCommand(command);
    }

    renderTrigger += 2;

    // Make sure changing keyboard focus is handled.
    Widget::handleCommand(command);
}

void LineInputWidget::handleKeyboardEvent(GUI::KeyboardEvent const &event) noexcept
{
    Widget::handleKeyboardEvent(event);

    if (!enabled) {
        return;
    }

    // This lock is held during rendering, only update the field when holding this lock.
    auto lock = std::scoped_lock(guiMutex);

    switch (event.type) {
    case GUI::KeyboardEvent::Type::Grapheme:
        field.insertGrapheme(event.grapheme);
        break;

    case GUI::KeyboardEvent::Type::PartialGrapheme:
        field.insertPartialGrapheme(event.grapheme);
        break;

    default:;
    }

    renderTrigger += 2;
}

void LineInputWidget::handleMouseEvent(GUI::MouseEvent const &event) noexcept {
    Widget::handleMouseEvent(event);

    if (!enabled) {
        return;
    }

    if (event.type == GUI::MouseEvent::Type::ButtonDown && event.cause.leftButton) {
        auto textRectangle = expand(box.currentRectangle(), -5.0f);
        if (textRectangle.contains(event.position)) {
            let textPosition = event.position - textRectangle.offset();

            if (event.down.shiftKey) {
                field.dragCursorAtCoordinate(textPosition);
            } else {
                if (event.clickCount == 1) {
                    field.setCursorAtCoordinate(textPosition);
                } else if (event.clickCount == 2) {
                    field.selectWordAtCoordinate(textPosition);
                }
            }
        }
        renderTrigger += 2;

    } else if (event.type == GUI::MouseEvent::Type::Move && event.down.leftButton) {
        auto textRectangle = expand(box.currentRectangle(), -5.0f);
        if (textRectangle.contains(event.position)) {
            let textPosition = event.position - textRectangle.offset();

            if (event.clickCount == 1) {
                field.dragCursorAtCoordinate(textPosition);
            } else if (event.clickCount == 2) {
                field.dragWordAtCoordinate(textPosition);
            }
        }
        renderTrigger += 2;
    }
}

HitBox LineInputWidget::hitBoxTest(vec position) noexcept
{
    if (box.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::TextEdit : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
