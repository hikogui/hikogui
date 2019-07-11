// Copyright 2019 Pokitec
// All rights reserved.

#include "ToolbarButtonWidget.hpp"
#include "utils.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace std::literals;

ToolbarButtonWidget::ToolbarButtonWidget(Draw::Path icon, std::function<void()> delegate) :
    Widget(), delegate(delegate)
{
    icon.tryRemoveLayers();
    this->icon = std::move(icon);
}

void ToolbarButtonWidget::setParent(Widget *parent)
{
    Widget::setParent(parent);

    window->addConstraint(box.height == box.width);
}

int ToolbarButtonWidget::state() const {
    int r = 0;
    r |= window->active ? 1 : 0;
    r |= hover ? 2 : 0;
    r |= pressed ? 4 : 0;
    r |= enabled ? 8 : 0;
    return r;
}


void ToolbarButtonWidget::pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex>& vertices, size_t& offset)
{
    auto vulkanDevice = device();

    if (!window->resizing) {
        currentExtent = box.currentExtent();
    }
    let currentScale = box.currentExtent() / currentExtent;

    key.update("ToolbarButtonWidget", currentExtent, this, state());

    vulkanDevice->imagePipeline->exchangeImage(image, key, currentExtent);

    drawImage(*image);

    GUI::PipelineImage::ImageLocation location;
    location.depth = depth + 0.0f;
    location.origin = {0.0, 0.0};
    location.position = box.currentPosition() + location.origin;
    location.scale = currentScale;
    location.rotation = 0.0;
    location.alpha = 1.0;
    location.clippingRectangle = box.currentRectangle();

    image->placeVertices(location, vertices, offset);
}

void ToolbarButtonWidget::drawImage(GUI::PipelineImage::Image &image)
{
    if (image.drawn) {
        return;
    }

    auto vulkanDevice = device();

    auto linearMap = Draw::PixelMap<wsRGBA>{image.extent};
    if (pressed) {
        fill(linearMap, pressedBackgroundColor);
    } else if (hover && enabled) {
        fill(linearMap, hoverBackgroundColor);
    } else {
        fill(linearMap);
    }

    let iconSize = boost::numeric_cast<float>(image.extent.height());
    let iconLocation = glm::vec2{image.extent.width() / 2.0f, image.extent.height() / 2.0f};

    auto iconImage = Draw::PixelMap<wsRGBA>{image.extent};
    if (std::holds_alternative<Draw::Path>(icon)) {
        let p = Draw::Alignment::MiddleCenter + T2D(iconLocation, iconSize) * Draw::PathString{std::get<Draw::Path>(icon)};

        fill(iconImage);
        composit(iconImage, p.toPath(wsRGBA{ 0xffffffff }), Draw::SubpixelOrientation::RedLeft);
    } else {
        no_default;
    }

    if (!(hover || window->active)) {
        desaturate(iconImage, 0.5f);
    }

    composit(linearMap, iconImage);

    auto pixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(image.extent);
    fill(pixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(image);
    image.drawn = true;
}

void ToolbarButtonWidget::handleMouseEvent(MouseEvent event) {
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
