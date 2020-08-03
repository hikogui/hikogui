// Copyright 2019 Pokitec
// All rights reserved.

#include "ButtonWidget.hpp"
#include "../GUI/utils.hpp"
#include "../utils.hpp"
#include <cmath>
#include <typeinfo>

namespace tt {

using namespace std::literals;

ButtonWidget::ButtonWidget(Window &window, Widget *parent, std::string const label) noexcept :
    Widget(window, parent),
    label(label)
{
}

ButtonWidget::~ButtonWidget() {
}

[[nodiscard]] bool ButtonWidget::updateConstraints() noexcept
{
    if (!Widget::updateConstraints()) {
        return false;
    }

    ttlet lock = std::scoped_lock(mutex);

    labelCell = std::make_unique<TextCell>(label, theme->warningLabelStyle);

    window.replaceConstraint(minimumWidthConstraint, width >= labelCell->preferredExtent().width() + Theme::margin * 2.0f);
    window.replaceConstraint(maximumWidthConstraint, width <= labelCell->preferredExtent().width() + Theme::margin * 2.0f, rhea::strength::weak());
    window.replaceConstraint(minimumHeightConstraint, height >= labelCell->preferredExtent().height() + Theme::margin * 2.0f);
    window.replaceConstraint(maximumHeightConstraint, height <= labelCell->preferredExtent().height() + Theme::margin * 2.0f, rhea::strength::weak());

    window.replaceConstraint(baseConstraint, base == middle);
    return true;
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
    context.drawBoxIncludeBorder(rectangle());

    if (*enabled) {
        context.color = theme->foregroundColor;
    }
    context.transform = drawContext.transform * (mat::T{0.0f, 0.0f, 0.001f} * textTranslate);
    labelCell->draw(context, rectangle(), Alignment::MiddleCenter, baseHeight(), true);

    Widget::draw(drawContext, displayTimePoint);
}

void ButtonWidget::handleCommand(command command) noexcept {
    if (!*enabled) {
        return;
    }

    if (command == command::gui_activate) {
        if (assign_and_compare(value, !value)) {
            window.requestRedraw = true;
        }
    }
    Widget::handleCommand(command);
}

void ButtonWidget::handleMouseEvent(MouseEvent const &event) noexcept {
    Widget::handleMouseEvent(event);

    if (*enabled) {
        if (assign_and_compare(pressed, static_cast<bool>(event.down.leftButton))) {
            window.requestRedraw = true;
        }

        if (
            event.type == MouseEvent::Type::ButtonUp &&
            event.cause.leftButton &&
            rectangle().contains(event.position)
        ) {
            handleCommand(command::gui_activate);
        }
    }
}

HitBox ButtonWidget::hitBoxTest(vec position) const noexcept
{
    if (rectangle().contains(position)) {
        return HitBox{this, elevation, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
