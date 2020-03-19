// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/WindowTrafficLightsWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
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

bool WindowTrafficLightsWidget::updateAndPlaceVertices(
    bool modified,
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineBox::Vertex> &box_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    ttauri_assert(window);
    backingImage.loadOrDraw(*window, box.currentExtent(), [&](auto image) {
        return drawImage(image);
        }, "WindowTrafficLightsWidget", state());

    if (backingImage.image) {
        let currentScale = (box.currentExtent() / vec{backingImage.image->extent}).xy10();

        GUI::PipelineImage::ImageLocation location;
        let T = mat::T(box.currentOffset(depth));
        let S = mat::S(currentScale);
        location.transform = T * S;
        location.clippingRectangle = box.currentRectangle();

        backingImage.image->placeVertices(location, image_vertices);
    }

    return Widget::updateAndPlaceVertices(modified, flat_vertices, box_vertices, image_vertices, sdf_vertices);
}

void WindowTrafficLightsWidget::drawTrianglesOutward(Path &path, vec position, float radius) noexcept
{
    let L = radius * 0.5;
    let W = radius * 0.3;

    path.moveTo({position.x() - L, position.y() - L});
    path.lineTo({position.x() + W, position.y() - L});
    path.lineTo({position.x() - L, position.y() + W});
    path.closeContour();

    path.moveTo({position.x() + L, position.y() + L});
    path.lineTo({position.x() - W, position.y() + L});
    path.lineTo({position.x() + L, position.y() - W});
    path.closeContour();
}

void WindowTrafficLightsWidget::drawTrianglesInward(Path &path, vec position, float radius) noexcept
{
    let L = radius * 0.8;

    path.moveTo({position.x(), position.y()});
    path.lineTo({position.x() - L, position.y()});
    path.lineTo({position.x(), position.y() - L});
    path.closeContour();

    path.moveTo({position.x(), position.y()});
    path.lineTo({position.x() + L, position.y()});
    path.lineTo({position.x(), position.y() + L});
    path.closeContour();
}

void WindowTrafficLightsWidget::drawCross(Path &path, vec position, float radius) noexcept
{
    let W = sqrt(0.5);
    let L = radius * 0.5;
    
    // Left bottom line.
    path.moveTo({position.x() - W, position.y()});
    path.lineTo({position.x() - L, position.y() - L + W});
    path.lineTo({position.x() - L + W, position.y() - L});
    path.lineTo({position.x(), position.y() - W});

    // Right bottom line.
    path.lineTo({position.x() + L - W, position.y() - L});
    path.lineTo({position.x() + L, position.y() - L + W});
    path.lineTo({position.x() + W, position.y()});

    // Right top line.
    path.lineTo({position.x() + L, position.y() + L - W});
    path.lineTo({position.x() + L - W, position.y() + L});
    path.lineTo({position.x(), position.y() + W});

    // Left top line.
    path.lineTo({position.x() - L + W, position.y() + L});
    path.lineTo({position.x() - L, position.y() + L - W});

    path.closeContour();
}

PixelMap<R16G16B16A16SFloat> WindowTrafficLightsWidget::drawApplicationIconImage(PipelineImage::Image &image) noexcept
{
    auto linearMap = PixelMap<R16G16B16A16SFloat>{image.extent};
    fill(linearMap);

    let iconPath = applicationIcon.centerScale(vec{image.extent}, 3.0);
    composit(linearMap, iconPath);

    if (!window->active) {
        desaturate(linearMap, 0.5f);
    }
    return linearMap;
}

PixelMap<R16G16B16A16SFloat> WindowTrafficLightsWidget::drawTrafficLightsImage(PipelineImage::Image &image) noexcept
{
    let height = box.height.value();

    let redCenter = vec{
        MARGIN + RADIUS,
        height / 2.0
    };
    let yellowCenter = vec{
        MARGIN + DIAMETER + SPACING + RADIUS,
        height / 2.0
    };
    let greenCenter = vec{
        MARGIN + DIAMETER + SPACING + DIAMETER + SPACING + RADIUS,
        height / 2.0
    };

    auto drawing = Path();
    drawing.addCircle(redCenter, RADIUS);
    
    if (!window->active && !hover) {
        drawing.closeLayer(vec{0.246, 0.246, 0.246, 1.0});
    } else if (pressedRed) {
        drawing.closeLayer(vec{1.0, 0.242, 0.212, 1.0});
    } else {
        drawing.closeLayer(vec{1.0, 0.1, 0.082, 1.0});
    }

    drawing.addCircle(yellowCenter, RADIUS);
    if (!window->active && !hover) {
        drawing.closeLayer(vec{0.246, 0.246, 0.246, 1.0});
    } else if (pressedYellow) {
        drawing.closeLayer(vec{1.0, 0.847, 0.093, 1.0});
    } else {
        drawing.closeLayer(vec{0.784, 0.521, 0.021, 1.0});
    }

    drawing.addCircle(greenCenter, RADIUS);
    if (!window->active && !hover) {
        drawing.closeLayer(vec{0.246, 0.246, 0.246, 1.0});
    } else if (pressedGreen) {
        drawing.closeLayer(vec{0.223, 0.863, 0.1, 1.0});
    } else {
        drawing.closeLayer(vec{0.082, 0.533, 0.024, 1.0});
    }

    if (hover) {
        drawCross(drawing, redCenter, RADIUS);
        drawing.closeLayer(vec{0.319, 0.0, 0.0, 1.0});

        drawing.addRectangle({yellowCenter.x() - RADIUS * 0.5 - 0.5, yellowCenter.y() - 0.5, RADIUS * 1.0 + 1.0, 1.0});
        drawing.closeLayer(vec{0.212, 0.1, 0.0, 1.0});

        if (window->size == Window::Size::Maximized) {
            drawTrianglesInward(drawing, greenCenter, RADIUS);
        } else {
            drawTrianglesOutward(drawing, greenCenter, RADIUS);
        }
        drawing.closeLayer(vec{0.0, 0.133, 0.0, 1.0});
    }

    auto linearMap = PixelMap<R16G16B16A16SFloat>{image.extent};
    fill(linearMap);
    composit(linearMap, drawing);
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

std::tuple<rect, rect, rect, rect> WindowTrafficLightsWidget::getButtonRectangles() const noexcept
{
    let left = box.left.value();
    let bottom = box.bottom.value();
    let height = box.height.value();

    let sysmenuButtonBox = rect{
        {left, bottom},
        {height, height}
    };

    let redButtonBox = rect{
        {left + MARGIN, bottom + MARGIN},
        {DIAMETER, DIAMETER}
    };

    let yellowButtonBox = rect{
        {left + MARGIN + DIAMETER + SPACING, bottom + MARGIN},
        {DIAMETER, DIAMETER}
    };

    let greenButtonBox = rect{
        {left + MARGIN + DIAMETER * 2.0 + SPACING * 2.0, bottom + MARGIN},
        {DIAMETER, DIAMETER}
    };

    return {redButtonBox, yellowButtonBox, greenButtonBox, sysmenuButtonBox};    
}

bool WindowTrafficLightsWidget::handleMouseEvent(MouseEvent const &event) noexcept
{
    bool r = false;

    window->setCursor(Cursor::Clickable);

    if constexpr (operatingSystem == OperatingSystem::Windows) {
        return r;

    } else if constexpr (operatingSystem == OperatingSystem::MacOS) {
        // Due to HitBox checking by Windows 10, every time cursor is on a
        // non client-area the WM_MOUSELEAVE event is send to the window.
        // The WM_MOUSELEAVE event does not include the mouse position,
        // neither inside the window, nor on the screen.
        // We can therefor not determine that the mouse is on the Widget.
        r |= assign_and_compare(hover, event.type != MouseEvent::Type::Exited);

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
        r |= assign_and_compare(pressedRed, event.down.leftButton && redButtonRect.contains(event.position));
        r |= assign_and_compare(pressedYellow, event.down.leftButton && yellowButtonRect.contains(event.position));
        r |= assign_and_compare(pressedGreen, event.down.leftButton && greenButtonRect.contains(event.position));

    } else {
        no_default;
    }

    return r;
}

HitBox WindowTrafficLightsWidget::hitBoxTest(vec position) const noexcept
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
