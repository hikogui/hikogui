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
    Widget(window, parent, vec{Theme::width, Theme::height}),
    label(std::move(label))
{
}

ButtonWidget::~ButtonWidget() {
}

void ButtonWidget::layout(hires_utc_clock::time_point displayTimePoint) noexcept 
{
    Widget::layout(displayTimePoint);

    let label_x = Theme::margin;
    let label_y = 0.0;
    let label_width = rectangle.width() - Theme::margin * 2.0f;
    let label_height = rectangle.height();
    let label_rectangle = aarect{label_x, label_y, label_width, label_height};

    labelShapedText = ShapedText(label, theme->warningLabelStyle, label_width + 1.0f, Alignment::MiddleCenter);
    textTranslate = labelShapedText.T(label_rectangle);

    setMinimumExtent(vec{Theme::width, labelShapedText.boundingBox.height() + Theme::margin * 2.0f});

    setPreferedExtent(labelShapedText.preferedExtent + Theme::margin2D * 2.0f);
}

void ButtonWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    auto context = drawContext;

    context.cornerShapes = vec{Theme::roundingRadius};
    if (value) {
        context.fillColor = theme->accentColor;
    }

    // Move the border of the button in the middle of a pixel.
    context.transform = drawContext.transform;
    context.drawBoxIncludeBorder(rectangle);

    if (enabled) {
        context.color = theme->foregroundColor;
    }
    context.transform = drawContext.transform * (mat::T{0.0f, 0.0f, 0.001f} * textTranslate);
    context.drawTextSingleColor(labelShapedText);

    Widget::draw(drawContext, displayTimePoint);
}

void ButtonWidget::handleCommand(string_ltag command) noexcept {
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

void ButtonWidget::handleMouseEvent(GUI::MouseEvent const &event) noexcept {
    Widget::handleMouseEvent(event);

    if (enabled) {
        if (assign_and_compare(pressed, static_cast<bool>(event.down.leftButton))) {
            forceRedraw = true;
        }

        if (
            event.type == GUI::MouseEvent::Type::ButtonUp &&
            event.cause.leftButton &&
            rectangle.contains(event.position)
        ) {
            handleCommand("gui.activate"_ltag);
        }
    }
}

HitBox ButtonWidget::hitBoxTest(vec position) const noexcept
{
    if (rectangle.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
