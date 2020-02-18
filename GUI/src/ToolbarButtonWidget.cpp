// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/ToolbarButtonWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace std::literals;

ToolbarButtonWidget::ToolbarButtonWidget(Path icon, std::function<void()> delegate) noexcept :
    Widget(), delegate(delegate)
{
    icon.tryRemoveLayers();
    this->icon = std::move(icon);
}

void ToolbarButtonWidget::setParent(Widget *parent) noexcept
{
    Widget::setParent(parent);

    window->addConstraint(box.height == box.width);
}

int ToolbarButtonWidget::state() const noexcept {
    int r = 0;
    r |= window->active ? 1 : 0;
    r |= hover ? 2 : 0;
    r |= pressed ? 4 : 0;
    r |= enabled ? 8 : 0;
    return r;
}


void ToolbarButtonWidget::update(
    bool modified,
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    ttauri_assert(window);
    backingImage.loadOrDraw(*window, box.currentExtent(), [&](auto image) {
        return drawImage(image);
    }, "ToolbarButtonWidget", this, state());

    if (backingImage.image) {
        let currentScale = box.currentExtent() / extent2{backingImage.image->extent};

        GUI::PipelineImage::ImageLocation location;
        location.depth = depth + 0.0f;
        location.origin = {0.0, 0.0};
        location.position = box.currentPosition() + location.origin;
        location.scale = currentScale;
        location.rotation = 0.0;
        location.alpha = 1.0;
        location.clippingRectangle = box.currentRectangle();

        backingImage.image->placeVertices(location, image_vertices);
    }

    Widget::update(modified, flat_vertices, image_vertices, sdf_vertices);
}

PipelineImage::Backing::ImagePixelMap ToolbarButtonWidget::drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept
{
    auto linearMap = PixelMap<wsRGBA>{image->extent};
    if (pressed) {
        fill(linearMap, pressedBackgroundColor);
    } else if (hover && enabled) {
        fill(linearMap, hoverBackgroundColor);
    } else {
        fill(linearMap);
    }

    let iconSize = numeric_cast<float>(image->extent.height());
    let iconLocation = glm::vec2{image->extent.width() / 2.0f, 0.0f};

    auto iconImage = PixelMap<wsRGBA>{image->extent};
    if (std::holds_alternative<Path>(icon)) {
        auto p = std::get<Path>(icon).centerScale(static_cast<extent2>(image->extent), 10.0);
        p.closeLayer(wsRGBA{1.0, 1.0, 1.0, 1.0});

        fill(iconImage);
        composit(iconImage, p, SubpixelOrientation::RedLeft);
    } else {
        no_default;
    }

    if (!(hover || window->active)) {
        desaturate(iconImage, 0.5f);
    }

    composit(linearMap, iconImage);
    return { std::move(image), std::move(linearMap) };
}

void ToolbarButtonWidget::handleMouseEvent(MouseEvent event) noexcept {
    hover = event.type != MouseEvent::Type::Exited;

    if (enabled) {
        window->setCursor(Cursor::Clickable);
        pressed = event.down.leftButton;

        if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            delegate();
        }

    } else {
        window->setCursor(Cursor::Default);
    }
}

}
