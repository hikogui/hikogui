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
    ControlWidget(window, parent, vec{Theme::width, Theme::height}), label(std::move(label))
{
}

ButtonWidget::~ButtonWidget() {
}



void ButtonWidget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    let rectangle = aarect{vec{}, box.currentExtent()};

    if (renderTrigger.check(displayTimePoint) >= 2) {
        let labelWidth = rectangle.width() - Theme::margin * 2.0f;

        LOG_DEBUG("Shaping button label {}", to_string(theme->warningLabelStyle));
        labelShapedText = ShapedText(label, theme->warningLabelStyle, HorizontalAlignment::Center, labelWidth);

        setMinimumExtent(Theme::maxLabelWidth, Theme::height);
        //setMinimumExtent(Theme::width, labelShapedText.extent.height());
        //setPreferedExtent(labelShapedText.preferedExtent);
    }

    auto context = drawContext;

    context.cornerShapes = vec{Theme::roundingRadius};
    if (value) {
        context.fillColor = theme->accentColor;
    }

    // Move the border of the button in the middle of a pixel.
    context.transform = drawContext.transform * mat::R(0.08726646259971647);
    context.drawBox(rectangle);

    auto textOffset = rectangle.align(labelShapedText.extent, Alignment::MiddleCenter);

    context.transform = drawContext.transform * mat::T{textOffset.z(0.001f)} * mat::R(0.08726646259971647);
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
