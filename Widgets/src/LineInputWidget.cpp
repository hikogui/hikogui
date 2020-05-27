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
    Widget(window, parent, vec{Theme::width, Theme::height}),
    label(std::move(label)),
    field(theme->labelStyle),
    shapedText()
{
}

LineInputWidget::~LineInputWidget()
{
}

int LineInputWidget::needs(hires_utc_clock::time_point displayTimePoint) noexcept
{
    auto need = Widget::needs(displayTimePoint);

    bool redraw = focus;
    redraw &= displayTimePoint > nextRedrawTimePoint;

    need |= static_cast<int>(redraw);

    return need;
}

void LineInputWidget::layout(hires_utc_clock::time_point displayTimePoint) noexcept 
{
    Widget::layout(displayTimePoint);

    textRectangle = shrink(rectangle(), Theme::margin);

    // Set the clipping rectangle to within the border of the input field.
    // Add another border width, so glyphs do not touch the border.
    textClippingRectangle = shrink(windowRectangle(), Theme::borderWidth * 2.0f);

    field.setStyleOfAll(theme->labelStyle);

    if (ssize(field) == 0) {
        shapedText = ShapedText(label, theme->placeholderLabelStyle, textRectangle.width(), Alignment::TopLeft);
    } else {
        field.setWidth(textRectangle.width());
        shapedText = field.shapedText();
    }

    setFixedHeight(shapedText.boundingBox.height() + Theme::margin * 2.0f);

    // Record the last time the text is modified, so that the carret remains lit.
    lastUpdateTimePoint = displayTimePoint;
}

void LineInputWidget::dragSelect() noexcept
{
    let mouseInTextPosition = textInvTranslate * dragSelectPosition;
    switch (dragClickCount) {
    case 1:
        field.dragCursorAtCoordinate(mouseInTextPosition);
        break;
    case 2:
        field.dragWordAtCoordinate(mouseInTextPosition);
        break;
    case 3:
        field.dragParagraphAtCoordinate(mouseInTextPosition);
        break;
    default:;
    }
}

void LineInputWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    nextRedrawTimePoint = displayTimePoint + blinkInterval;

    auto context = drawContext;

    context.drawBoxIncludeBorder(rectangle());

    // After drawing the border around the input field make sure any other
    // drawing remains inside this border.
    context.clippingRectangle = textClippingRectangle;

    if (dragScrollSpeedX != 0.0f) {
        textScrollX += dragScrollSpeedX * (1.0f/60.0f);
        dragSelect();

        // Once we are scrolling, don't stop.
        forceRedraw = true;

    } else if (dragClickCount == 0) {
        // The following is for scrolling based on keyboard input, ignore mouse drags.

        // Scroll the text a quarter width to the left until the cursor is within the width
        // of the text field
        if (leftToRightCaret.x() - textScrollX > textRectangle.width()) {
            textScrollX = leftToRightCaret.x() - textRectangle.width() * 0.75f;
        }

        // Scroll the text a quarter width to the right until the cursor is within the width
        // of the text field
        while (leftToRightCaret.x() - textScrollX < 0.0f) {
            textScrollX = leftToRightCaret.x() - textRectangle.width() * 0.25f;
        }
    }

    // cap how far we scroll.
    let maxScrollWidth = std::max(0.0f, shapedText.preferedExtent.width() - textRectangle.width());
    textScrollX = std::clamp(textScrollX, 0.0f, maxScrollWidth);

    textTranslate = mat::T2(-textScrollX, 0.0f) * shapedText.T(textRectangle);
    textInvTranslate = ~textTranslate;

    context.transform = drawContext.transform * (mat::T(0.0, 0.0, 0.0001f) * textTranslate);

    selectionRectangles = field.selectionRectangles();
    for (let selectionRectangle: selectionRectangles) {
        context.fillColor = theme->textSelectColor;
        context.drawFilledQuad(selectionRectangle);
    }

    partialGraphemeCaret = field.partialGraphemeCaret();
    if (partialGraphemeCaret) {
        context.fillColor = theme->incompleteGlyphColor;
        context.drawFilledQuad(partialGraphemeCaret);
    }

    // Display the caret and handle blinking.
    auto durationSinceLastUpdate = displayTimePoint - lastUpdateTimePoint;
    auto nrHalfBlinks = static_cast<int64_t>(durationSinceLastUpdate / blinkInterval);

    auto blinkIsOn = nrHalfBlinks % 2 == 0;
    leftToRightCaret = field.leftToRightCaret();
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

    forceLayout = true;

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

    forceLayout = true;
}

void LineInputWidget::handleMouseEvent(GUI::MouseEvent const &event) noexcept {
    Widget::handleMouseEvent(event);

    // Make sure we only scroll when dragging outside the widget.
    dragScrollSpeedX = 0.0f;
    dragClickCount = event.clickCount;
    dragSelectPosition = event.position;

    if (!enabled) {
        return;
    }

    if (event.type == GUI::MouseEvent::Type::ButtonDown && event.cause.leftButton) {
        if (textRectangle.contains(event.position)) {
            let mouseInTextPosition = textInvTranslate * event.position;

            switch (event.clickCount) {
            case 1:
                if (event.down.shiftKey) {
                    field.dragCursorAtCoordinate(mouseInTextPosition);
                } else {
                    field.setCursorAtCoordinate(mouseInTextPosition);
                }
                break;
            case 2:
                field.selectWordAtCoordinate(mouseInTextPosition);
                break;
            case 3:
                field.selectParagraphAtCoordinate(mouseInTextPosition);
                break;
            default:;
            }
        }

        // Record the last time the cursor is moved, so that the carret remains lit.
        lastUpdateTimePoint = event.timePoint;

        forceRedraw = true;

    } else if (event.type == GUI::MouseEvent::Type::Drag && event.cause.leftButton) {
        // When the mouse is dragged beyond the line input,
        // start scrolling the text and select on the edge of the textRectangle.
        if (event.position.x() > textRectangle.p2().x()) {
            // The mouse is on the right of the text.
            dragSelectPosition.x(textRectangle.p2().x());

            // Scroll text to the left in points per second.
            dragScrollSpeedX = 50.0f;

        } else if (event.position.x() < textRectangle.x()) {
            // The mouse is on the left of the text.
            dragSelectPosition.x(textRectangle.x());

            // Scroll text to the right in points per second.
            dragScrollSpeedX = -50.0f;
        }

        dragSelect();

        forceRedraw = true;
    }
}

HitBox LineInputWidget::hitBoxTest(vec position) const noexcept
{
    if (rectangle().contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::TextEdit : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
