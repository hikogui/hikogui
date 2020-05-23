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
{}

WidgetNeed LineInputWidget::needs(hires_utc_clock::time_point displayTimePoint) const noexcept
{
    auto need = Widget::needs(displayTimePoint);

    bool redraw = focus;
    redraw &= displayTimePoint > nextRedrawTimePoint;

    need |= static_cast<WidgetNeed>(redraw);

    return need;
}

void LineInputWidget::layout(hires_utc_clock::time_point displayTimePoint) noexcept 
{
    Widget::layout(displayTimePoint);

    textRectangle = shrink(rectangle, Theme::margin);

    field.setStyleOfAll(theme->labelStyle);

    field.setExtent(textRectangle.extent());
    leftToRightCaret = field.leftToRightCaret();
    partialGraphemeCaret = field.partialGraphemeCaret();
    selectionRectangles = field.selectionRectangles();

    if (ssize(field) == 0) {
        shapedText = ShapedText(label, theme->placeholderLabelStyle, Alignment::TopLeft, textRectangle.width());

    } else {
        shapedText = field.shapedText();
    }

    // Record the last time the text is modified, so that the carret remains lit.
    lastUpdateTimePoint = displayTimePoint;
}

void LineInputWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    nextRedrawTimePoint = displayTimePoint + blinkInterval;

    auto context = drawContext;

    context.drawBoxIncludeBorder(rectangle);

    auto textTranslate = shapedText.T(textRectangle);
    context.transform = context.transform * (mat::T(0.0, 0.0, 0.0001f) * textTranslate);

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
    auto nrHalfBlinks = static_cast<int64_t>(durationSinceLastUpdate / blinkInterval);

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

    if (!enabled) {
        return;
    }

    if (event.type == GUI::MouseEvent::Type::ButtonDown && event.cause.leftButton) {
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
        forceLayout = true;

    } else if (event.type == GUI::MouseEvent::Type::Move && event.down.leftButton) {
        if (textRectangle.contains(event.position)) {
            let textPosition = event.position - textRectangle.offset();

            if (event.clickCount == 1) {
                field.dragCursorAtCoordinate(textPosition);
            } else if (event.clickCount == 2) {
                field.dragWordAtCoordinate(textPosition);
            }
        }
        forceLayout = true;
    }
}

HitBox LineInputWidget::hitBoxTest(vec position) const noexcept
{
    if (rectangle.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::TextEdit : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
