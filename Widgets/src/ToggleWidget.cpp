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

ToggleWidget::ToggleWidget(Window &window, Widget *parent, observed<bool> &value, std::string const label) noexcept :
    Widget(window, parent), value(150ms, value, [this](bool){ ++this->renderTrigger; }), label(std::move(label))
{
    if (ssize(label) != 0) {
        window.addConstraint(box.width >= Theme::width);
    } else {
        window.addConstraint(box.width >= Theme::smallWidth);
    }
    window.addConstraint(box.height >= Theme::smallHeight);
}

void ToggleWidget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    // Prepare animation values.
    let [animation_progress, curr_value] = value.animation_tick(displayTimePoint);
    if (animation_progress < 1.0) {
        ++renderTrigger;
    }

    // Prepare coordinates.
    let rectangle = box.currentRectangle();

    let toggle_height = Theme::smallHeight;
    let toggle_width = Theme::smallWidth + 1.0f; // Expand horizontally due to rounded shape
    let toggle_x = -0.5f;  // Expand horizontally due to rounded shape
    let toggle_y = (rectangle.height() - toggle_height) * 0.5f;
    let toggle_rectangle = rect{toggle_x, toggle_y, toggle_width, toggle_height};

    let slider_move = toggle_width - toggle_height;
    let slider_x = toggle_x + Theme::borderWidth + slider_move * curr_value;
    let slider_y = toggle_y + Theme::borderWidth;
    let slider_width = toggle_height - 2.0f * Theme::borderWidth;
    let slider_height = toggle_height - 2.0f * Theme::borderWidth;
    let slider_rectangle = rect{slider_x, slider_y, slider_width, slider_height};

    let label_x = Theme::smallWidth + theme->margin;
    let label_y = 0.0f;
    let label_width = rectangle.width() - label_x;
    let label_height = rectangle.height();
    let label_rectangle = rect{label_x, label_y, label_width, label_height};

    // Prepare labels.
    if (renderTrigger.check(displayTimePoint) >= 2) {
        labelShapedText = ShapedText(label, theme->labelStyle, HorizontalAlignment::Left, label_width);
    }
    let label_translate = mat::T{label_rectangle.align(labelShapedText.extent, Alignment::MiddleLeft)};

    // Outside oval.
    auto context = drawContext;
    context.cornerShapes = vec{toggle_rectangle.height() * 0.5f};
    context.drawBox(toggle_rectangle);

    // Inside circle
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
    context.drawBox(slider_rectangle);

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
            ++renderTrigger;
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

HitBox ToggleWidget::hitBoxTest(vec position) noexcept
{
    if (box.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
