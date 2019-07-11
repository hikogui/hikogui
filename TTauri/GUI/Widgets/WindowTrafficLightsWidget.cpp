// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowTrafficLightsWidget.hpp"
#include "utils.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

WindowTrafficLightsWidget::WindowTrafficLightsWidget(Draw::Path applicationIcon) :
    Widget(), applicationIcon(std::move(applicationIcon))
{
}

void WindowTrafficLightsWidget::setParent(Widget *parent)
{
    Widget::setParent(parent);

    this->window->addConstraint(box.height == HEIGHT);
    this->window->addConstraint(box.width == WIDTH);
}

int WindowTrafficLightsWidget::state() const {
    int r = 0;
    r |= window->active ? 1 : 0;
    if constexpr (operatingSystem == OperatingSystem::MacOS) {
        r |= hover ? 2 : 0;
        r |= pressedRed ? 4 : 0;
        r |= pressedYellow ? 8 : 0;
        r |= pressedGreen ? 16 : 0;
        r |= (window->size == Window::Size::Maximized) ? 32 : 0;
    }
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

void WindowTrafficLightsWidget::drawApplicationIconImage(PipelineImage::Image &image)
{
    auto vulkanDevice = device();

    auto linearMap = Draw::PixelMap<wsRGBA>{image.extent};
    fill(linearMap);

    let iconSize = boost::numeric_cast<float>(image.extent.height());
    let iconLocation = glm::vec2{image.extent.width() / 2.0f, image.extent.height() / 2.0f};
    let iconString = Draw::Alignment::MiddleCenter + T2D(iconLocation, iconSize) * Draw::PathString{applicationIcon};

    fill(linearMap);
    composit(linearMap, iconString.toPath(wsRGBA{ 0xffffffff }), window->subpixelOrientation);

    if (!window->active) {
        desaturate(linearMap, 0.5f);
    }

    auto pixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(image.extent);
    fill(pixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(image);
}

void WindowTrafficLightsWidget::drawTrafficLightsImage(PipelineImage::Image &image)
{
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

    composit(linearMap, drawing, window->subpixelOrientation);

    auto pixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(image.extent);
    fill(pixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(image);
}

void WindowTrafficLightsWidget::drawImage(PipelineImage::Image &image)
{
    if (image.drawn) {
        return;
    }

    if constexpr (operatingSystem == OperatingSystem::Windows10) {
        drawApplicationIconImage(image);
    } else if constexpr (operatingSystem == OperatingSystem::MacOS) {
        drawTrafficLightsImage(image);
    } else {
        no_default;
    }

    image.drawn = true;
}

std::tuple<rect2, rect2, rect2, rect2> WindowTrafficLightsWidget::getButtonRectangles() const
{
    let left = box.left.value();
    let bottom = box.bottom.value();
    let height = box.height.value();

    let sysmenuButtonBox = rect2{
        {left, bottom},
        {height, height}
    };

    let redButtonBox = rect2{
        {left + MARGIN, bottom + MARGIN},
        {DIAMETER, DIAMETER}
    };

    let yellowButtonBox = rect2{
        {left + MARGIN + DIAMETER + SPACING, bottom + MARGIN},
        {DIAMETER, DIAMETER}
    };

    let greenButtonBox = rect2{
        {left + MARGIN + DIAMETER * 2.0 + SPACING * 2.0, bottom + MARGIN},
        {DIAMETER, DIAMETER}
    };

    return {redButtonBox, yellowButtonBox, greenButtonBox, sysmenuButtonBox};    
}

void WindowTrafficLightsWidget::handleMouseEvent(MouseEvent event)
{
    window->setCursor(Cursor::Clickable);

    if constexpr (operatingSystem == OperatingSystem::Windows10) {
        return;

    } else if constexpr (operatingSystem == OperatingSystem::MacOS) {
        // Due to HitBox checking by Windows 10, every time cursor is on a
        // non client-area the WM_MOUSELEAVE event is send to the window.
        // The WM_MOUSELEAVE event does not include the mouse position,
        // neither inside the window, nor on the screen.
        // We can therefor not determine that the mouse is on the Widget.
        hover = event.type != MouseEvent::Type::Exited;

        let [redButtonRect, yellowButtonRect, greenButtonRect, sysmenuButtonBox] = getButtonRectangles();

        if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            if (pressedRed) {
                window->closeWindow();
            } else if (pressedYellow) {
                window->minimizeWindow();
            } else if (pressedGreen) {
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

        // Only change the pressed state after checking for Button Up, the
        // button up will check which button was pressed from button down.
        pressedRed = false;
        pressedYellow = false;
        pressedGreen = false;
        if (event.down.leftButton) {
            if (redButtonRect.contains(event.position)) {
                pressedRed = true;
            } else if (yellowButtonRect.contains(event.position)) {
                pressedYellow = true;
            } else if (greenButtonRect.contains(event.position)) {
                pressedGreen = true;
            }
        }

    } else {
        no_default;
    }
}

HitBox WindowTrafficLightsWidget::hitBoxTest(glm::vec2 position) const
{
    let [redButtonRect, yellowButtonRect, greenButtonRect, sysmenuButtonBox] = getButtonRectangles();

    if constexpr (operatingSystem == OperatingSystem::Windows10) {
        if (sysmenuButtonBox.contains(position)) {
            return HitBox::ApplicationIcon;
        } else {
            return HitBox::MoveArea;
        }

    } else if constexpr (operatingSystem == OperatingSystem::MacOS) {
        if (redButtonRect.contains(position) ||
            yellowButtonRect.contains(position) ||
            greenButtonRect.contains(position)
        ) {
            return HitBox::NoWhereInteresting;
        } else {
            return HitBox::MoveArea;
        }

    } else {
        no_default;
    }
}

}
