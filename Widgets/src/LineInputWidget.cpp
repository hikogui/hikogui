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

LineInputWidget::LineInputWidget(Window &window, Widget *parent, std::string const label, TextStyle style) noexcept :
    Widget(window, parent),
    label(std::move(label)),
    field(style),
    shapedText()
{
}

void LineInputWidget::draw(DrawContext &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    auto context = drawContext;
    context.clippingRectangle = expand(box.currentRectangle(), 10.0);

    // Draw something.
    context.cornerShapes = vec{0.0, 0.0, 0.0, 0.0};

    if (hover || focus) {
        context.fillColor = vec::color(0.1, 0.1, 0.1);
    } else {
        context.fillColor = vec::color(0.01, 0.01, 0.01);
    }

    if (focus) {
        context.borderColor = vec::color(0.072, 0.072, 1.0);
    } else if (hover) {
        context.borderColor = vec::color(0.2, 0.2, 0.2);
    } else {
        context.borderColor = vec::color(0.1, 0.1, 0.1);
    }

    context.color = vec{1.0, 1.0, 1.0, 1.0};
    context.transform = mat::T(0.0, 0.0, elevation);
    context.drawBox(box.currentRectangle());

    auto textRectangle = expand(box.currentRectangle(), -5.0f);
    if (renderTrigger.check(displayTimePoint) >= 2) {
        field.setExtent(textRectangle.extent());
        leftToRightCaret = field.leftToRightCaret();
        partialGraphemeCaret = field.partialGraphemeCaret();
        selectionRectangles = field.selectionRectangles();

        if (ssize(field) == 0) {
            let labelStyle = TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 14.0, context.color, 0.0, TextDecoration::None);
            shapedText = ShapedText(label, labelStyle, HorizontalAlignment::Left, textRectangle.width());

        } else {
            shapedText = field.shapedText();
        }

        window.device->SDFPipeline->prepareAtlas(shapedText);

        lastUpdateTimePoint = displayTimePoint;
    }

    context.clippingRectangle = box.currentRectangle();
    context.transform = mat::T(textRectangle.offset(elevation + 0.0002f));
    context.drawText(shapedText);
   
    context.transform = mat::T(textRectangle.offset(elevation + 0.0001f));
    for (let selectionRectangle: selectionRectangles) {
        context.fillColor = vec::color(0.0, 0.0, 1.0);
        context.drawFilledQuad(selectionRectangle);
    }

    if (partialGraphemeCaret) {
        context.fillColor = vec::color(0.2, 0.2, 0.0);
        context.drawFilledQuad(partialGraphemeCaret);
    }

    // Display the caret and handle blinking.
    auto durationSinceLastUpdate = displayTimePoint - lastUpdateTimePoint;
    auto nrHalfBlinks = static_cast<int64_t>(durationSinceLastUpdate / 500ms);

    auto nextHalfBlinkTime = lastUpdateTimePoint + ((nrHalfBlinks + 1) * 500ms);
    renderTrigger += nextHalfBlinkTime;

    auto blinkIsOn = nrHalfBlinks % 2 == 0;
    if (leftToRightCaret && blinkIsOn && focus && window.active) {
        context.fillColor = vec::color(0.5, 0.5, 0.5);
        context.drawFilledQuad(leftToRightCaret);
    }

    Widget::draw(drawContext, displayTimePoint);
}


void LineInputWidget::handleCommand(string_ltag command) noexcept
{
    LOG_DEBUG("LineInputWidget: Received command: {}", tt5_decode(command));
    if (!enabled) {
        return;
    }

    // This lock is held during rendering, only update the field when holding this lock.
    std::scoped_lock lock(GUI_globals->mutex);

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
    std::scoped_lock lock(GUI_globals->mutex);

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
