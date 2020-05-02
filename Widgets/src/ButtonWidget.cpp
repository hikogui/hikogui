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
    minimumExtent = vec{Theme::width, Theme::height};
    minimumWidthConstraint = window.addConstraint(box.width >= minimumExtent.width());
    minimumHeightConstraint = window.addConstraint(box.height >= minimumExtent.height());
}

ButtonWidget::~ButtonWidget() {
    window.removeConstraint(minimumWidthConstraint);
    window.removeConstraint(minimumHeightConstraint);
}

void ButtonWidget::setMinimumExtent(vec newMinimumExtent) noexcept {
    if (newMinimumExtent != minimumExtent) {
        minimumExtent = newMinimumExtent;

        minimumWidthConstraint = window.replaceConstraint(
            minimumWidthConstraint,
            box.width >= minimumExtent.width()
        );

        minimumHeightConstraint = window.replaceConstraint(
            minimumHeightConstraint,
            box.height >= minimumExtent.height()
        );
    }
}

void ButtonWidget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    if (renderTrigger.check(displayTimePoint) >= 2) {
        labelShapedText = ShapedText(label, theme->labelStyle, HorizontalAlignment::Center, Theme::maxLabelWidth);
        window.device->SDFPipeline->prepareAtlas(labelShapedText);

        // Add minimumSize extent to the Widget, then only when the minimumSize changes due to fontShaping
        // should the constraints be modified.
        //
        //window.removeConstraint(widthConstraint);
        //widthConstraint = window.addConstraint(box.width >= labelShapedText.extent.width());
    }

    auto context = drawContext;

    context.cornerShapes = vec{Theme::roundingRadius};
    if (value) {
        context.fillColor = theme->accentColor;
    }

    // Move the border of the button in the middle of a pixel.
    let buttonRectangle = rect{vec{}, box.currentExtent()};
    context.drawBox(buttonRectangle);

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
