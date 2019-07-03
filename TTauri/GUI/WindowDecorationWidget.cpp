// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowDecorationWidget.hpp"
#include "PipelineImage_Image.hpp"
#include "PipelineImage_Vertex.hpp"
#include "Device.hpp"
#include "TTauri/Draw/PixelMap.inl"
#include <cmath>
#include <boost/math/constants/constants.hpp>
#include <typeinfo>

namespace TTauri::GUI {

using namespace std::literals;

WindowDecorationWidget::WindowDecorationWidget(Draw::Alignment alignment) :
    alignment(alignment), Widget()
{
}

void WindowDecorationWidget::setParent(Widget *parent)
{
    Widget::setParent(parent);

    //this->window->addConstraint(box.height >= 20);
    this->window->addConstraint(box.outerBottom() == parent->box.bottom);
    this->window->addConstraint(box.outerTop() == parent->box.top());

    switch (alignment) {
    case Draw::Alignment::TopLeft:
        this->window->addConstraint(box.outerLeft() == parent->box.left);
        this->window->addConstraint(box.width == box.height * 1);
        break;
    case Draw::Alignment::TopRight:
        this->window->addConstraint(box.outerRight() == parent->box.right());
        this->window->addConstraint(box.width == box.height * 3);
        break;
    default:
        no_default;
    }
}

void WindowDecorationWidget::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex>& vertices, size_t& offset)
{
    auto vulkanDevice = device();

    if (!window->resizing) {
        currentExtent = box.currentExtent();
    }
    let currentScale = box.currentExtent() / currentExtent;

    key.update("WindowDecorationWidget", currentExtent, static_cast<int>(alignment), state());

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

void WindowDecorationWidget::drawImage(PipelineImage::Image &image)
{
    if (image.drawn) {
        return;
    }

    auto vulkanDevice = device();

    auto linearMap = Draw::PixelMap<wsRGBA>{image.extent};
    fill(linearMap, wsRGBA{ 0xffffff88 });

    auto pixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(image.extent);
    fill(pixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(image);
    image.drawn = true;
}

void WindowDecorationWidget::handleMouseEvent(MouseEvent event) {
    if (enabled) {
        window->setCursor(GUI::Cursor::Clickable);
        pressed = event.down.leftButton;

        if (event.type == GUI::MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            value = !value;
        }

    } else {
        window->setCursor(GUI::Cursor::Default);
    }
}

}
