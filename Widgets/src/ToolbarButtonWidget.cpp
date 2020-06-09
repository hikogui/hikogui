// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ToolbarButtonWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri {

using namespace std::literals;

ToolbarButtonWidget::ToolbarButtonWidget(Window &window, Widget *parent, icon_type icon, std::function<void()> delegate) noexcept :
    Widget(window, parent, vec{Theme::smallWidth, Theme::smallHeight}),
    icon(std::move(icon)),
    delegate(delegate)
{
}


void ToolbarButtonWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    // Draw background of button.
    {
        auto context = drawContext;

        if (pressed) {
            context.fillColor = closeButton ? vec::color(1.0, 0.0, 0.0) : theme->fillColor(nestingLevel() + 1);
        } else if (hover && enabled) {
            context.fillColor = closeButton ? vec::color(0.5, 0.0, 0.0) : theme->fillColor(nestingLevel());
        } else {
            context.fillColor = theme->fillColor(nestingLevel() - 1);
        }
        context.drawFilledQuad(rectangle());
    }

    if (auto icon_glyph = std::get_if<Text::FontGlyphIDs>(&icon)) {
        auto context = drawContext;
        context.color = theme->foregroundColor;

        let buttonBox = shrink(aarect{extent()}, Theme::margin);

        let glyphBoundingBox = PipelineSDF::DeviceShared::getBoundingBox(*icon_glyph);

        let glyphRectangle = align(buttonBox, scale(glyphBoundingBox, Theme::iconSize), Alignment::MiddleCenter);

        context.drawGlyph(*icon_glyph, glyphRectangle);
    } else {
        no_default;
    }

    Widget::draw(drawContext, displayTimePoint);
}

void ToolbarButtonWidget::handleMouseEvent(MouseEvent const &event) noexcept {
    Widget::handleMouseEvent(event);

    if (enabled) {
        if (assign_and_compare(pressed, static_cast<bool>(event.down.leftButton))) {
            forceRedraw = true;
        }

        if (
            event.type == MouseEvent::Type::ButtonUp &&
            event.cause.leftButton &&
            rectangle().contains(event.position)
        ) {
            delegate();
        }
    }
}

HitBox ToolbarButtonWidget::hitBoxTest(vec position) const noexcept
{
    if (rectangle().contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
