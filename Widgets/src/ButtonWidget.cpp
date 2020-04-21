// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ButtonWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace TTauri::Text;
using namespace std::literals;

ButtonWidget::ButtonWidget(Window &window, Widget *parent, std::string const label) noexcept :
    Widget(window, parent), label(std::move(label))
{
}

void ButtonWidget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    auto context = drawContext;

    context.cornerShapes = theme->buttonCornerShapes;
    if (value) {
        if (hover) {
            context.fillColor = theme->accentColor;
        } else if (pressed) {
            context.fillColor = theme->fillColor(nestingLevel() + 1);
        } else {
            context.fillColor = theme->accentColor;
        }
    } else {
        if (hover) {
            context.fillColor = theme->fillColor(nestingLevel() + 1);
        } else if (pressed) {
            context.fillColor = theme->accentColor;
        } else {
            context.fillColor = theme->fillColor(nestingLevel());
        }
    }

    if (focus) {
        context.color = theme->accentColor;
    } else {
        context.color = theme->fillColor(nestingLevel() + 1);
    }

    context.lineWidth = theme->buttonBorderWidth;

    let buttonRectangle = shrink(rect{vec{}, box.currentExtent()}, theme->padding);
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

void ButtonWidget::handleCommand(string_ltag command) noexcept {
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

void ButtonWidget::handleMouseEvent(GUI::MouseEvent const &event) noexcept {
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

HitBox ButtonWidget::hitBoxTest(vec position) noexcept
{
    if (box.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
