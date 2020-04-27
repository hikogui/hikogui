// Copyright 2019 Pokitec
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
    Widget(window, parent), value(250ms, value, [this](bool){ ++this->renderTrigger; }), label(std::move(label))
{
}

void ToggleWidget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    auto context = drawContext;

    let [animation_progress, curr_value] = value.animation_tick(displayTimePoint);
    if (animation_progress < 1.0) {
        ++renderTrigger;
    }

    context.cornerShapes = theme->buttonCornerShapes;
    if (hover) {
        context.fillColor = mix(curr_value, theme->fillColor(nestingLevel() + 1), theme->accentColor);
    } else if (pressed) {
        context.fillColor = mix(curr_value, theme->accentColor, theme->fillColor(nestingLevel() + 1));
    } else {
        context.fillColor = mix(curr_value, theme->fillColor(nestingLevel()), theme->accentColor);
    }

    if (focus) {
        context.color = theme->accentColor;
    } else {
        context.color = theme->fillColor(nestingLevel() + 1);
    }

    context.lineWidth = theme->buttonBorderWidth;

    // Move the border of the button in the middle of a pixel.
    let buttonRectangle = shrink(rect{vec{}, box.currentExtent()}, 0.5);
    context.drawBox(buttonRectangle);

    if (renderTrigger.check(displayTimePoint) >= 2) {
        labelShapedText = ShapedText(label, theme->labelStyle, HorizontalAlignment::Center, buttonRectangle.width());

        window.device->SDFPipeline->prepareAtlas(labelShapedText);
    }

    auto textOffset = buttonRectangle.align(labelShapedText.extent, Alignment::MiddleCenter);

    context.transform = context.transform * mat::T{textOffset.z(0.001f)};
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
        if (assign_and_compare(pressed, static_cast<bool>(event.down.leftButton))) {
            ++renderTrigger;
        }

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
