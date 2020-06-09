// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ToggleWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include "TTauri/Foundation/math.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri {

using namespace TTauri::Text;
using namespace std::literals;


void ToggleWidget::layout(hires_utc_clock::time_point displayTimePoint) noexcept
{
    Widget::layout(displayTimePoint);

    // The label is located to the right of the toggle.
    let label_x = Theme::smallWidth + Theme::margin;
    label_rectangle = aarect{
        label_x, 0.0f,
        rectangle().width() - label_x, rectangle().height()
    };

    labelShapedText = ShapedText(label, theme->labelStyle, label_rectangle.width(), Alignment::TopLeft);
    label_translate = labelShapedText.T(label_rectangle);
    setFixedHeight(std::max(labelShapedText.boundingBox.height(), Theme::smallHeight));

    toggle_height = Theme::smallHeight;
    toggle_width = Theme::smallWidth + 1.0f; // Expand horizontally due to rounded shape
    toggle_x = -0.5f;  // Expand horizontally due to rounded shape
    toggle_y = rectangle().height() - toggle_height;
    toggle_rectangle = aarect{toggle_x, toggle_y, toggle_width, toggle_height};
    
    slider_x = 1.5f;
    slider_y = toggle_y + 1.5f;
    slider_width = toggle_height - 3.0f;
    slider_height = toggle_height - 3.0f;
    let slider_move_width = Theme::smallWidth - (slider_x * 2.0f);
    slider_move = slider_move_width - slider_width;

}

void ToggleWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    // Prepare animation values.
    let [animation_progress, curr_value] = value.animation_tick(displayTimePoint);
    if (animation_progress < 1.0) {
        forceRedraw = true;
    }

    // Outside oval.
    auto context = drawContext;
    context.cornerShapes = vec{toggle_rectangle.height() * 0.5f};
    context.drawBoxIncludeBorder(toggle_rectangle);

    // Inside circle
    let slider_rectangle = aarect{
        slider_x + slider_move * curr_value, slider_y,
        slider_width, slider_height
    };

    if (value) {
        if (enabled && window.active) {
            context.color = theme->accentColor;
        }
    } else {
        if (enabled && window.active) {
            context.color = hover ?
                theme->borderColor(nestingLevel() + 1) :
                theme->borderColor(nestingLevel());
        }
    }
    std::swap(context.color, context.fillColor);
    context.cornerShapes = vec{slider_rectangle.height() * 0.5f};
    context.drawBoxIncludeBorder(slider_rectangle);

    // user defined label.
    context.transform = drawContext.transform * label_translate * mat::T{0.0, 0.0, 0.001f};
    context.drawText(labelShapedText);

    Widget::draw(drawContext, displayTimePoint);
}

void ToggleWidget::handleCommand(string_ltag command) noexcept {
    if (!enabled) {
        return;
    }

    if (command == "gui.activate"_ltag) {
        if (assign_and_compare(value, !value)) {
            forceRedraw = true;
        }
    }
    Widget::handleCommand(command);
}

void ToggleWidget::handleMouseEvent(MouseEvent const &event) noexcept {
    Widget::handleMouseEvent(event);

    if (enabled) {
        if (
            event.type == MouseEvent::Type::ButtonUp &&
            event.cause.leftButton &&
            rectangle().contains(event.position)
        ) {
            handleCommand("gui.activate"_ltag);
        }
    }
}

HitBox ToggleWidget::hitBoxTest(vec position) const noexcept
{
    if (rectangle().contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
