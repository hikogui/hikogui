// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "LineInputWidget.hpp"
#include "../GUI/utils.hpp"
#include "../utils.hpp"
#include <cmath>
#include <typeinfo>

namespace tt {

using namespace std::literals;

LineInputWidget::LineInputWidget(Window &window, std::shared_ptr<widget> parent, std::u8string const label) noexcept :
    widget(window, parent), label(std::move(label)), field(theme->labelStyle), shapedText()
{
}

LineInputWidget::~LineInputWidget() {}

bool LineInputWidget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_assume(gui_system_mutex.recurse_lock_count());

    if (super::update_constraints(display_time_point, need_reconstrain)) {
        ttlet maximumHeight = shapedText.boundingBox.height() + Theme::margin * 2.0f;

        _preferred_size = {
            vec{100.0f, Theme::smallSize + Theme::margin * 2.0f},
            vec{500.0f, Theme::smallSize + Theme::margin * 2.0f}
        };
        _preferred_base_line = relative_base_line{vertical_alignment::middle, 0.0f, 200.0f};
        return true;
    } else {
        return false;
    }
}

bool LineInputWidget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(gui_system_mutex.recurse_lock_count());

    auto need_redraw = need_layout |= std::exchange(_request_relayout, false);
    need_redraw |= _focus && display_time_point >= nextRedrawTimePoint;
    if (need_layout) {
        textRectangle = shrink(rectangle(), Theme::margin);

        // Set the clipping rectangle to within the border of the input field.
        // Add another border width, so glyphs do not touch the border.
        textClippingRectangle = intersect(_window_clipping_rectangle, shrink(_window_rectangle, Theme::borderWidth * 2.0f));

        field.setStyleOfAll(theme->labelStyle);

        if (std::ssize(field) == 0) {
            shapedText = ShapedText(label, theme->placeholderLabelStyle, textRectangle.width(), alignment::middle_left);
        } else {
            field.setWidth(textRectangle.width());
            shapedText = field.shapedText();
        }

        // Record the last time the text is modified, so that the caret remains lit.
        lastUpdateTimePoint = display_time_point;
    }

    return widget::update_layout(display_time_point, need_layout) || need_redraw;
}

void LineInputWidget::dragSelect() noexcept
{
    tt_assume(gui_system_mutex.recurse_lock_count());

    ttlet mouseInTextPosition = textInvTranslate * dragSelectPosition;
    switch (dragClickCount) {
    case 1: field.dragCursorAtCoordinate(mouseInTextPosition); break;
    case 2: field.dragWordAtCoordinate(mouseInTextPosition); break;
    case 3: field.dragParagraphAtCoordinate(mouseInTextPosition); break;
    default:;
    }
}

void LineInputWidget::scrollText() noexcept
{
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
    ttlet maxScrollWidth = std::max(0.0f, shapedText.preferred_extent.width() - textRectangle.width());
    textScrollX = std::clamp(textScrollX, 0.0f, maxScrollWidth);

    // Calculate how much we need to translate the text.
    textTranslate = mat::T2(-textScrollX, 0.0f) * shapedText.T(textRectangle);
    textInvTranslate = ~textTranslate;
}

void LineInputWidget::drawBackgroundBox(DrawContext const &context) const noexcept
{
    context.drawBoxIncludeBorder(rectangle());
}

void LineInputWidget::drawSelectionRectangles(DrawContext context) const noexcept
{
    ttlet selectionRectangles = field.selectionRectangles();
    for (ttlet selectionRectangle : selectionRectangles) {
        context.fillColor = theme->textSelectColor;
        context.drawFilledQuad(selectionRectangle);
    }
}

void LineInputWidget::drawPartialGraphemeCaret(DrawContext context) const noexcept
{
    ttlet partialGraphemeCaret = field.partialGraphemeCaret();
    if (partialGraphemeCaret) {
        context.fillColor = theme->incompleteGlyphColor;
        context.drawFilledQuad(partialGraphemeCaret);
    }
}

void LineInputWidget::drawCaret(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
{
    // Display the caret and handle blinking.
    auto durationSinceLastUpdate = display_time_point - lastUpdateTimePoint;
    auto nrHalfBlinks = static_cast<int64_t>(durationSinceLastUpdate / blinkInterval);

    auto blinkIsOn = nrHalfBlinks % 2 == 0;
    leftToRightCaret = field.leftToRightCaret();
    if (leftToRightCaret && blinkIsOn && _focus && window.active) {
        context.fillColor = theme->cursorColor;
        context.drawFilledQuad(leftToRightCaret);
    }
}

void LineInputWidget::drawText(DrawContext context) const noexcept
{
    context.transform = mat::T(0.0f, 0.0f, 0.2f) * context.transform;
    context.drawText(shapedText);
}

void LineInputWidget::draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_assume(gui_system_mutex.recurse_lock_count());

    nextRedrawTimePoint = display_time_point + blinkInterval;
    scrollText();

    drawBackgroundBox(context);

    // After drawing the border around the input field make sure any other
    // drawing remains inside this border. And change the transform to account
    // for how much the text has scrolled.
    context.clippingRectangle = textClippingRectangle;
    context.transform = (mat::T(0.0, 0.0, 0.1f) * textTranslate) * context.transform;

    drawSelectionRectangles(context);
    drawPartialGraphemeCaret(context);
    drawCaret(context, display_time_point);
    drawText(context);

    widget::draw(std::move(context), display_time_point);
}

bool LineInputWidget::handle_command(command command) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    auto handled = widget::handle_command(command);

    LOG_DEBUG("LineInputWidget: Received command: {}", command);
    if (*enabled) {
        switch (command) {
        case command::text_edit_paste:
            handled = true;
            field.handlePaste(window.getTextFromClipboard());
            break;

        case command::text_edit_copy:
            handled = true;
            window.setTextOnClipboard(field.handleCopy());
            break;

        case command::text_edit_cut:
            handled = true;
            window.setTextOnClipboard(field.handleCut());
            break;

        default:
            handled |= field.handle_command(command);
        }
    }

    _request_relayout = true;
    return handled;
}

bool LineInputWidget::handle_keyboard_event(KeyboardEvent const &event) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    auto handled = widget::handle_keyboard_event(event);

    if (*enabled) {
        switch (event.type) {
        case KeyboardEvent::Type::Grapheme:
            handled = true;
            field.insertGrapheme(event.grapheme);
            break;

        case KeyboardEvent::Type::PartialGrapheme:
            handled = true;
            field.insertPartialGrapheme(event.grapheme);
            break;

        default:;
        }
    }

    _request_relayout = true;
    return handled;
}

bool LineInputWidget::handle_mouse_event(MouseEvent const &event) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    auto handled = widget::handle_mouse_event(event);

    // Make sure we only scroll when dragging outside the widget.
    ttlet position = _from_window_transform * event.position;
    dragScrollSpeedX = 0.0f;
    dragClickCount = event.clickCount;
    dragSelectPosition = position;

    if (event.cause.leftButton) {
        handled = true;

        if (!*enabled) {
            return true;
        }

        switch (event.type) {
        using enum MouseEvent::Type;
        case ButtonDown:
            if (textRectangle.contains(position)) {
                ttlet mouseInTextPosition = textInvTranslate * position;

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

                // Record the last time the cursor is moved, so that the caret remains lit.
                lastUpdateTimePoint = event.timePoint;

                window.requestRedraw = true;

            }
            break;

        case Drag:
            // When the mouse is dragged beyond the line input,
            // start scrolling the text and select on the edge of the textRectangle.
            if (position.x() > textRectangle.p3().x()) {
                // The mouse is on the right of the text.
                dragSelectPosition.x(textRectangle.p3().x());

                // Scroll text to the left in points per second.
                dragScrollSpeedX = 50.0f;

            } else if (position.x() < textRectangle.x()) {
                // The mouse is on the left of the text.
                dragSelectPosition.x(textRectangle.x());

                // Scroll text to the right in points per second.
                dragScrollSpeedX = -50.0f;
            }

            dragSelect();

            window.requestRedraw = true;
            break;

        default:;
        }
    }
    return handled;
}

HitBox LineInputWidget::hitbox_test(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    if (_window_clipping_rectangle.contains(window_position)) {
        return HitBox{weak_from_this(), _draw_layer, *enabled ? HitBox::Type::TextEdit : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

} // namespace tt
