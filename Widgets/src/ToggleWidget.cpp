// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ToggleWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include "TTauri/Foundation/math.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace TTauri::Text;
using namespace std::literals;


void ToggleWidget::layout(hires_utc_clock::time_point displayTimePoint) noexcept
{
    Widget::layout(displayTimePoint);

    toggle_height = Theme::smallHeight;
    toggle_width = Theme::smallWidth + 1.0f; // Expand horizontally due to rounded shape
    toggle_x = -0.5f;  // Expand horizontally due to rounded shape
    toggle_y = (rectangle.height() - toggle_height) * 0.5f;
    toggle_rectangle = aarect{toggle_x, toggle_y, toggle_width, toggle_height};
    
    slider_x = 1.5f;
    slider_y = 1.5f;
    slider_width = toggle_height - 3.0f;
    slider_height = toggle_height - 3.0f;
    let slider_move_width = Theme::smallWidth - (slider_x * 2.0f);
    slider_move = slider_move_width - slider_width;

    label_x = Theme::smallWidth + theme->margin;
    label_y = 0.0f;
    label_width = rectangle.width() - label_x;
    label_height = rectangle.height();
    label_rectangle = aarect{label_x, label_y, label_width, label_height};

    labelShapedText = ShapedText(label, theme->labelStyle, label_width, Alignment::MiddleLeft);
    label_translate = labelShapedText.T(label_rectangle);
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

    if (enabled) {
        if (value) {
            context.fillColor = theme->accentColor;
        } else if (hover) {
            context.fillColor = theme->borderColor(nestingLevel() + 1);
        } else {
            context.fillColor = theme->borderColor(nestingLevel());
        }
    } else {
        context.fillColor = theme->borderColor(nestingLevel() - 1);
    }
    context.color = vec::color(0.0, 0.0, 0.0, 0.0);
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

void ToggleWidget::handleMouseEvent(GUI::MouseEvent const &event) noexcept {
    Widget::handleMouseEvent(event);

    if (enabled) {
        if (event.type == GUI::MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            handleCommand("gui.activate"_ltag);
        }
    }
}

HitBox ToggleWidget::hitBoxTest(vec position) const noexcept
{
    if (rectangle.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
