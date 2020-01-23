// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/WindowTrafficLightsWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

WindowTrafficLightsWidget::WindowTrafficLightsWidget(Path applicationIcon) noexcept :
    Widget(), applicationIcon(std::move(applicationIcon))
{
}

void WindowTrafficLightsWidget::setParent(Widget *parent) noexcept
{
    Widget::setParent(parent);

    this->window->addConstraint(box.height == HEIGHT);
    this->window->addConstraint(box.width == WIDTH);
}

int WindowTrafficLightsWidget::state() const noexcept {
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

void WindowTrafficLightsWidget::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex>& vertices, int& offset) noexcept
{
    ttauri_assert(window);
    backingImage.loadOrDraw(*window, box.currentExtent(), [&](auto image) {
        return drawImage(image);
        }, "WindowTrafficLightsWidget", state());

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

        backingImage.image->placeVertices(location, vertices, offset);
    }

    Widget::pipelineImagePlaceVertices(vertices, offset);
}

void WindowTrafficLightsWidget::drawTrianglesOutward(Path &path, glm::vec2 position, float radius) noexcept
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

void WindowTrafficLightsWidget::drawTrianglesInward(Path &path, glm::vec2 position, float radius) noexcept
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

void WindowTrafficLightsWidget::drawCross(Path &path, glm::vec2 position, float radius) noexcept
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

PixelMap<wsRGBA> WindowTrafficLightsWidget::drawApplicationIconImage(PipelineImage::Image &image) noexcept
{
    auto linearMap = PixelMap<wsRGBA>{image.extent};
    fill(linearMap);

    let iconPath = applicationIcon.centerScale(static_cast<extent2>(image.extent), 5.0);

    fill(linearMap);
    composit(linearMap, iconPath, window->subpixelOrientation);

    if (!window->active) {
        desaturate(linearMap, 0.5f);
    }
    return linearMap;
}

PixelMap<wsRGBA> WindowTrafficLightsWidget::drawTrafficLightsImage(PipelineImage::Image &image) noexcept
{
    auto linearMap = PixelMap<wsRGBA>{image.extent};
    fill(linearMap);

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

    auto drawing = Path();
    drawing.addCircle(redCenter, RADIUS);
    
    if (!window->active && !hover) {
        drawing.closeLayer(wsRGBA{ 0x888888ff });
    } else if (pressedRed) {
        drawing.closeLayer(wsRGBA{ 0xff877fff });
    } else {
        drawing.closeLayer(wsRGBA{ 0xff5951ff });
    }

    drawing.addCircle(yellowCenter, RADIUS);
    if (!window->active && !hover) {
        drawing.closeLayer(wsRGBA{ 0x888888ff });
    } else if (pressedYellow) {
        drawing.closeLayer(wsRGBA{ 0xffed56ff });
    } else {
        drawing.closeLayer(wsRGBA{ 0xe5bf28ff });
    }

    drawing.addCircle(greenCenter, RADIUS);
    if (!window->active && !hover) {
        drawing.closeLayer(wsRGBA{ 0x888888ff });
    } else if (pressedGreen) {
        drawing.closeLayer(wsRGBA{ 0x82ef59ff });
    } else {
        drawing.closeLayer(wsRGBA{ 0x51c12bff });
    }

    if (hover) {
        drawCross(drawing, redCenter, RADIUS);
        drawing.closeLayer(wsRGBA{ 0x990000ff });

        drawing.addRectangle({{yellowCenter.x - RADIUS * 0.5 - 0.5, yellowCenter.y - 0.5}, {RADIUS * 1.0 + 1.0, 1.0}});
        drawing.closeLayer(wsRGBA{ 0x7f5900ff });

        if (window->size == Window::Size::Maximized) {
            drawTrianglesInward(drawing, greenCenter, RADIUS);
        } else {
            drawTrianglesOutward(drawing, greenCenter, RADIUS);
        }
        drawing.closeLayer(wsRGBA{ 0x006600ff });
    }

    composit(linearMap, drawing, window->subpixelOrientation);
    return linearMap;
}

PipelineImage::Backing::ImagePixelMap WindowTrafficLightsWidget::drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept
{
    if constexpr (operatingSystem == OperatingSystem::Windows) {
        return { std::move(image), drawApplicationIconImage(*image) };
    } else if constexpr (operatingSystem == OperatingSystem::MacOS) {
        return { std::move(image), drawTrafficLightsImage(*image) };
    } else {
        no_default;
    }
}

std::tuple<rect2, rect2, rect2, rect2> WindowTrafficLightsWidget::getButtonRectangles() const noexcept
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

void WindowTrafficLightsWidget::handleMouseEvent(MouseEvent event) noexcept
{
    window->setCursor(Cursor::Clickable);

    if constexpr (operatingSystem == OperatingSystem::Windows) {
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

HitBox WindowTrafficLightsWidget::hitBoxTest(glm::vec2 position) const noexcept
{
    let [redButtonRect, yellowButtonRect, greenButtonRect, sysmenuButtonBox] = getButtonRectangles();

    if constexpr (operatingSystem == OperatingSystem::Windows) {
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
