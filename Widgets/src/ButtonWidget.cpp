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

    context.cornerShapes = vec{ 10.0, 10.0, -10.0, 0.0 };
    if (value) {
        if (hover) {
            context.fillColor = vec::color(0.3, 0.3, 1.0);
        } else if (pressed) {
            context.fillColor = vec::color(0.1, 0.1, 0.1);
        } else {
            context.fillColor = vec::color(0.072, 0.072, 1.0);
        }
    } else {
        if (hover) {
            context.fillColor = vec::color(0.3, 0.3, 0.3);
        } else if (pressed) {
            context.fillColor = vec::color(0.072, 0.072, 1.0);
        } else {
            context.fillColor = vec::color(0.1, 0.1, 0.1);
        }
    }

    if (focus) {
        context.borderColor = vec::color(0.072, 0.072, 1.0);
    } else {
        context.borderColor = vec::color(0.3, 0.3, 0.3);
    }
    context.borderSize = 1.0;

    context.color = vec{1.0, 1.0, 1.0, 1.0};

    if (value || pressed) {
        context.shadowSize = 0.0;
    } else {
        context.shadowSize = 6.0;
    }

    let padding = 5.0;
    let buttonRectangle = shrink(rect{vec{}, box.currentExtent()}, padding);
    context.drawBox(buttonRectangle);

    if (renderTrigger.check(displayTimePoint) >= 2) {
        let labelStyle = TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 14.0, context.color, 0.0, TextDecoration::None);

        labelShapedText = ShapedText(label, labelStyle, HorizontalAlignment::Center, buttonRectangle.width());

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
