// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowTrafficLightsWidget.hpp"
#include "PipelineImage_Image.hpp"
#include "PipelineImage_Vertex.hpp"
#include "Device.hpp"
#include "TTauri/Draw/PixelMap.inl"
#include "TTauri/Draw/Path.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>
#include <typeinfo>

namespace TTauri::GUI {

using namespace std::literals;

WindowTrafficLightsWidget::WindowTrafficLightsWidget() :
    Widget()
{
}

void WindowTrafficLightsWidget::setParent(Widget *parent)
{
    Widget::setParent(parent);

    this->window->addConstraint(box.height == DIAMETER + 2.0 * MARGIN);
    this->window->addConstraint(box.width == DIAMETER * 3.0 + 2.0 * MARGIN + 2 * SPACING);

    this->window->addConstraint(box.outerBottom() >= parent->box.bottom);
    this->window->addConstraint(box.outerTop() == parent->box.top());
    this->window->addConstraint(box.outerLeft() == parent->box.left);
}

int WindowTrafficLightsWidget::state() const {
    int r = 0;
    r |= window->active ? 1 : 0;
    r |= hover ? 2 : 0;
    r |= pressedRed ? 4 : 0;
    r |= pressedYellow ? 8 : 0;
    r |= pressedGreen ? 16 : 0;
    r |= (window->size == Window::Size::Maximized) ? 32 : 0;
    return r;
}

void WindowTrafficLightsWidget::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex>& vertices, size_t& offset)
{
    auto vulkanDevice = device();

    if (!window->resizing) {
        currentExtent = box.currentExtent();
    }
    let currentScale = box.currentExtent() / currentExtent;

    key.update("WindowTrafficLightsWidget", currentExtent, state());

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

void WindowTrafficLightsWidget::drawTrianglesOutward(Draw::Path &path, glm::vec2 position, float radius)
{
    let L = radius * 0.5;
    let W = radius * 0.3;

    path.moveTo({position.x - L, position.y - L});
    path.lineTo({position.x + W, position.y - L});
    path.lineTo({position.x - L, position.y + W});
    path.closeContour();

    path.moveTo({position.x + L, position.y + L});
    path.lineTo({position.x - W, position.y + L});
    path.lineTo({position.x + L, position.y - W});
    path.closeContour();
}

void WindowTrafficLightsWidget::drawTrianglesInward(Draw::Path &path, glm::vec2 position, float radius)
{
    let L = radius * 0.8;

    path.moveTo({position.x, position.y});
    path.lineTo({position.x - L, position.y});
    path.lineTo({position.x, position.y - L});
    path.closeContour();

    path.moveTo({position.x, position.y});
    path.lineTo({position.x + L, position.y});
    path.lineTo({position.x, position.y + L});
    path.closeContour();
}

void WindowTrafficLightsWidget::drawCross(Draw::Path &path, glm::vec2 position, float radius)
{
    let W = sqrt(0.5);
    let L = radius * 0.5;
    
    // Left bottom line.
    path.moveTo({position.x - W, position.y});
    path.lineTo({position.x - L, position.y - L + W});
    path.lineTo({position.x - L + W, position.y - L});
    path.lineTo({position.x, position.y - W});

    // Right bottom line.
    path.lineTo({position.x + L - W, position.y - L});
    path.lineTo({position.x + L, position.y - L + W});
    path.lineTo({position.x + W, position.y});

    // Right top line.
    path.lineTo({position.x + L, position.y + L - W});
    path.lineTo({position.x + L - W, position.y + L});
    path.lineTo({position.x, position.y + W});

    // Left top line.
    path.lineTo({position.x - L + W, position.y + L});
    path.lineTo({position.x - L, position.y + L - W});

    path.closeContour();
}

void WindowTrafficLightsWidget::drawImage(PipelineImage::Image &image)
{
    if (image.drawn) {
        return;
    }

    auto vulkanDevice = device();

    auto linearMap = Draw::PixelMap<wsRGBA>{image.extent};
    fill(linearMap);

    let width = box.width.value();
    let buttonWidth = width / 3.0;
    let height = box.height.value();

    let redCenter = glm::vec2{
        MARGIN + RADIUS,
        height / 2.0
    };
    let yellowCenter = glm::vec2{
        MARGIN + DIAMETER + SPACING + RADIUS,
        height / 2.0
    };
    let greenCenter = glm::vec2{
        MARGIN + DIAMETER + SPACING + DIAMETER + SPACING + RADIUS,
        height / 2.0
    };

    auto drawing = Draw::Path();
    drawing.addCircle(redCenter, RADIUS);
    
    if (!window->active && !hover) {
        drawing.closeLayer({ 0x888888ff });
    } else if (pressedRed) {
        drawing.closeLayer({ 0xff877fff });
    } else {
        drawing.closeLayer({ 0xff5951ff });
    }

    drawing.addCircle(yellowCenter, RADIUS);
    if (!window->active && !hover) {
        drawing.closeLayer({ 0x888888ff });
    } else if (pressedYellow) {
        drawing.closeLayer({ 0xffed56ff });
    } else {
        drawing.closeLayer({ 0xe5bf28ff });
    }

    drawing.addCircle(greenCenter, RADIUS);
    if (!window->active && !hover) {
        drawing.closeLayer({ 0x888888ff });
    } else if (pressedGreen) {
        drawing.closeLayer({ 0x82ef59ff });
    } else {
        drawing.closeLayer({ 0x51c12bff });
    }

    if (hover) {
        drawCross(drawing, redCenter, RADIUS);
        drawing.closeLayer({ 0x990000ff });

        drawing.addRectangle({{yellowCenter.x - RADIUS * 0.5 - 0.5, yellowCenter.y - 0.5}, {RADIUS * 1.0 + 1.0, 1.0}});
        drawing.closeLayer({ 0x7f5900ff });

        if (window->size == Window::Size::Maximized) {
            drawTrianglesInward(drawing, greenCenter, RADIUS);
        } else {
            drawTrianglesOutward(drawing, greenCenter, RADIUS);
        }
        drawing.closeLayer({ 0x006600ff });
    }

    fill(linearMap, drawing, Draw::SubpixelOrientation::RedLeft);

    //fill(linearMap, wsRGBA{ 0xffffffff });

    auto pixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(image.extent);
    fill(pixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(image);
    image.drawn = true;
}


void WindowTrafficLightsWidget::handleMouseEvent(MouseEvent event)
{
    let left = box.left.value();

    window->setCursor(GUI::Cursor::Clickable);

    hover = event.type != MouseEvent::Type::Exited;

    if (event.down.leftButton) {
        if (event.position.x < (left + MARGIN + DIAMETER + SPACING / 2.0)) {
            pressedRed = true;
        } else if (event.position.x < (left + MARGIN + DIAMETER + SPACING + DIAMETER + SPACING / 2.0)) {
            pressedYellow = true;
        } else {
            pressedGreen = true;
        }

    } else {
        pressedRed = false;
        pressedYellow = false;
        pressedGreen = false;
    }

    if (event.type == GUI::MouseEvent::Type::ButtonUp && event.cause.leftButton) {
        if (event.position.x < (left + MARGIN + DIAMETER + SPACING / 2.0)) {
            window->closeWindow();
        } else if (event.position.x < (left + MARGIN + DIAMETER + SPACING + DIAMETER + SPACING / 2.0)) {
            window->minimizeWindow();
        } else {
            switch (window->size) {
            case Window::Size::Normal:
                window->maximizeWindow();
                break;
            case Window::Size::Maximized:
                window->normalizeWindow();
                break;
            default:
                no_default;
            }
        }
    }
}

}
