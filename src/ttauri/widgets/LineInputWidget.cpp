// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "LineInputWidget.hpp"
#include "../GUI/utils.hpp"
#include "../utils.hpp"
#include <cmath>
#include <typeinfo>

namespace tt {

using namespace std::literals;

LineInputWidget::LineInputWidget(Window &window, Widget *parent, std::u8string const label) noexcept :
    Widget(window, parent), label(std::move(label)), field(theme->labelStyle), shapedText()
{
}

LineInputWidget::~LineInputWidget() {}

bool LineInputWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (Widget::updateConstraints()) {
        ttlet maximumHeight = shapedText.boundingBox.height() + Theme::margin * 2.0f;

        _preferred_size = {
            vec{100.0f, Theme::smallSize + Theme::margin * 2.0f},
            vec{500.0f, Theme::smallSize + Theme::margin * 2.0f}
        };
        _preferred_base_line = relative_base_line{VerticalAlignment::Middle, 0.0f, 200.0f};
        return true;
    } else {
        return false;
    }
}

bool LineInputWidget::updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto need_redraw = need_layout |= requestLayout.exchange(false);
    need_redraw |= focus && display_time_point >= nextRedrawTimePoint;
    if (need_layout) {
        textRectangle = shrink(rectangle(), Theme::margin);

        // Set the clipping rectangle to within the border of the input field.
        // Add another border width, so glyphs do not touch the border.
        textClippingRectangle = shrink(window_rectangle(), Theme::borderWidth * 2.0f);

        field.setStyleOfAll(theme->labelStyle);

        if (std::ssize(field) == 0) {
            shapedText = ShapedText(label, theme->placeholderLabelStyle, textRectangle.width(), Alignment::MiddleLeft);
        } else {
            field.setWidth(textRectangle.width());
            shapedText = field.shapedText();
        }

        // Record the last time the text is modified, so that the caret remains lit.
        lastUpdateTimePoint = display_time_point;
    }

    return Widget::updateLayout(display_time_point, need_layout) || need_redraw;
}

void LineInputWidget::dragSelect() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    ttlet mouseInTextPosition = textInvTranslate * dragSelectPosition;
    switch (dragClickCount) {
    case 1: field.dragCursorAtCoordinate(mouseInTextPosition); break;
    case 2: field.dragWordAtCoordinate(mouseInTextPosition); break;
    case 3: field.dragParagraphAtCoordinate(mouseInTextPosition); break;
    default:;
    }
}

void LineInputWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    nextRedrawTimePoint = displayTimePoint + blinkInterval;

    auto context = drawContext;

    context.drawBoxIncludeBorder(rectangle());

    // After drawing the border around the input field make sure any other
    // drawing remains inside this border.
    context.clippingRectangle = textClippingRectangle;

    if (dragScrollSpeedX != 0.0f) {
        textScrollX += dragScrollSpeedX * (1.0f / 60.0f);
        dragSelect();

        // Once we are scrolling, don't stop.
        window.requestRedraw = true;

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
    ttlet maxScrollWidth = std::max(0.0f, shapedText.preferredExtent.width() - textRectangle.width());
    textScrollX = std::clamp(textScrollX, 0.0f, maxScrollWidth);

    textTranslate = mat::T2(-textScrollX, 0.0f) * shapedText.T(textRectangle);
    textInvTranslate = ~textTranslate;

    context.transform = drawContext.transform * (mat::T(0.0, 0.0, 0.0001f) * textTranslate);

    selectionRectangles = field.selectionRectangles();
    for (ttlet selectionRectangle : selectionRectangles) {
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

void LineInputWidget::handleCommand(command command) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    LOG_DEBUG("LineInputWidget: Received command: {}", command);
    if (!*enabled) {
        return;
    }

    switch (command) {
    case command::text_edit_paste: field.handlePaste(window.getTextFromClipboard()); break;
    case command::text_edit_copy: window.setTextOnClipboard(field.handleCopy()); break;
    case command::text_edit_cut: window.setTextOnClipboard(field.handleCut()); break;
    default: field.handleCommand(command);
    }

    requestLayout = true;

    // Make sure changing keyboard focus is handled.
    Widget::handleCommand(command);
}

void LineInputWidget::handleKeyboardEvent(KeyboardEvent const &event) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    Widget::handleKeyboardEvent(event);

    if (!*enabled) {
        return;
    }

    switch (event.type) {
    case KeyboardEvent::Type::Grapheme: field.insertGrapheme(event.grapheme); break;

    case KeyboardEvent::Type::PartialGrapheme: field.insertPartialGrapheme(event.grapheme); break;

    default:;
    }

    requestLayout = true;
}

void LineInputWidget::handleMouseEvent(MouseEvent const &event) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    Widget::handleMouseEvent(event);

    // Make sure we only scroll when dragging outside the widget.
    dragScrollSpeedX = 0.0f;
    dragClickCount = event.clickCount;
    dragSelectPosition = event.position;

    if (!*enabled) {
        return;
    }

    if (event.type == MouseEvent::Type::ButtonDown && event.cause.leftButton) {
        if (textRectangle.contains(event.position)) {
            ttlet mouseInTextPosition = textInvTranslate * event.position;

            switch (event.clickCount) {
            case 1:
                if (event.down.shiftKey) {
                    field.dragCursorAtCoordinate(mouseInTextPosition);
                } else {
                    field.setCursorAtCoordinate(mouseInTextPosition);
                }
                break;
            case 2: field.selectWordAtCoordinate(mouseInTextPosition); break;
            case 3: field.selectParagraphAtCoordinate(mouseInTextPosition); break;
            default:;
            }
        }

        // Record the last time the cursor is moved, so that the carret remains lit.
        lastUpdateTimePoint = event.timePoint;

        window.requestRedraw = true;

    } else if (event.type == MouseEvent::Type::Drag && event.cause.leftButton) {
        // When the mouse is dragged beyond the line input,
        // start scrolling the text and select on the edge of the textRectangle.
        if (event.position.x() > textRectangle.p3().x()) {
            // The mouse is on the right of the text.
            dragSelectPosition.x(textRectangle.p3().x());

            // Scroll text to the left in points per second.
            dragScrollSpeedX = 50.0f;

        } else if (event.position.x() < textRectangle.x()) {
            // The mouse is on the left of the text.
            dragSelectPosition.x(textRectangle.x());

            // Scroll text to the right in points per second.
            dragScrollSpeedX = -50.0f;
        }

        dragSelect();

        window.requestRedraw = true;
    }
}

HitBox LineInputWidget::hitBoxTest(vec position) const noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (rectangle().contains(position)) {
        return HitBox{this, elevation, *enabled ? HitBox::Type::TextEdit : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

} // namespace tt
