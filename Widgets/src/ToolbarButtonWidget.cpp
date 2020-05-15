// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ToolbarButtonWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace std::literals;

ToolbarButtonWidget::ToolbarButtonWidget(Window &window, Widget *parent, icon_type icon, std::function<void()> delegate) noexcept :
    Widget(window, parent), icon(std::move(icon)), delegate(delegate)
{
    window.addConstraint(box.height == box.width);

    if (auto path = std::get_if<Path>(&this->icon)) {
        path->tryRemoveLayers();
    }
}


int ToolbarButtonWidget::state() const noexcept {
    int r = 0;
    r |= window.active ? 1 : 0;
    r |= hover ? 2 : 0;
    r |= pressed ? 4 : 0;
    r |= enabled ? 8 : 0;
    return r;
}

PipelineImage::Backing::ImagePixelMap ToolbarButtonWidget::drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept
{
    auto iconImage = PixelMap<R16G16B16A16SFloat>{image->extent};
    if (std::holds_alternative<Path>(icon)) {
        auto p = std::get<Path>(icon).centerScale(vec{image->extent}, 10.0);
        p.closeLayer(vec::color(1.0, 1.0, 1.0));

        fill(iconImage);
        composit(iconImage, p);
    } else {
        no_default;
    }

    if (!(hover || window.active)) {
        desaturate(iconImage, 0.5f);
    }

    auto linearMap = PixelMap<R16G16B16A16SFloat>{image->extent};
    fill(linearMap);

    composit(linearMap, iconImage);
    return { std::move(image), std::move(linearMap) };
}

void ToolbarButtonWidget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
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
        context.drawFilledQuad(aarect{box.extent()});
    }

    if (std::holds_alternative<Path>(icon)) {
        auto drawingBackingImage = backingImage.loadOrDraw(
            window,
            box.extent(),
            [&](auto image) {
                return drawImage(image);
            },
            "ToolbarButtonWidget",
            this,
            state()
        );
        if (drawingBackingImage) {
            forceRedraw = true;
        }

        if (backingImage.image) {
            let currentScale = (box.extent() / vec{backingImage.image->extent}).xy11();

            auto context = drawContext;
            context.transform = context.transform * mat::S(currentScale);
            context.drawImage(*(backingImage.image));

            if (backingImage.image->state != PipelineImage::Image::State::Uploaded) {
                forceRedraw = true;
            }
        } else {
            forceRedraw = true;
        }

    } else if (auto icon_glyph = std::get_if<Text::FontGlyphIDs>(&icon)) {
        auto context = drawContext;
        context.color = theme->foregroundColor;

        let buttonBox = shrink(aarect{box.extent()}, Theme::margin);

        let glyphBoundingBox = PipelineSDF::DeviceShared::getBoundingBox(*icon_glyph);

        let box = align(buttonBox, scale(glyphBoundingBox, Theme::iconSize), Alignment::MiddleCenter);

        context.drawGlyph(*icon_glyph, box);
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

        if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            delegate();
        }
    }
}

HitBox ToolbarButtonWidget::hitBoxTest(vec position) noexcept
{
    if (box.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
