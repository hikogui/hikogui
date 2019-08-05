// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowToolbarWidget.hpp"
#include "WindowTrafficLightsWidget.hpp"
#include "ToolbarButtonWidget.hpp"
#include "utils.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>

namespace TTauri::GUI::Widgets {

using namespace std;

WindowToolbarWidget::WindowToolbarWidget() :
    Widget()
{
}

void WindowToolbarWidget::setParent(Widget *parent)
{
    Widget::setParent(parent);

    trafficLightButtons = addWidget<WindowTrafficLightsWidget>(
        getResource<Draw::Path>(URL("resource:Themes/Icons/Application Icon.tticon"))
    );
    window->addConstraint(trafficLightButtons->box.outerTop() == box.top());
    window->addConstraint(trafficLightButtons->box.outerLeft() == box.left);
    window->addConstraint(trafficLightButtons->box.outerBottom() == box.bottom);

    if constexpr (operatingSystem == OperatingSystem::Windows) {
        closeWindowButton = addWidget<ToolbarButtonWidget>(
            0.33f * getResource<Draw::Path>(URL("resource:Themes/Icons/Close%20Window.tticon")),
            [&]() { window->closeWindow(); }
        );
        closeWindowButton->hoverBackgroundColor = { 0xdd0000ff };
        closeWindowButton->pressedBackgroundColor = { 0xff0000ff };
        window->addConstraint(closeWindowButton->box.outerTop() == box.top());
        window->addConstraint(closeWindowButton->box.outerRight() == box.right());
        window->addConstraint(closeWindowButton->box.outerBottom() == box.bottom);

        maximizeWindowButton = addWidget<ToolbarButtonWidget>(
            0.33f * getResource<Draw::Path>(URL("resource:Themes/Icons/Maximize%20Window.tticon")),
            [&]() { 
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
        );
        window->addConstraint(maximizeWindowButton->box.outerTop() == box.top());
        window->addConstraint(maximizeWindowButton->box.outerRight() == closeWindowButton->box.outerLeft());
        window->addConstraint(maximizeWindowButton->box.outerBottom() == box.bottom);

        minimizeWindowButton = addWidget<ToolbarButtonWidget>(
            0.33f * getResource<Draw::Path>(URL("resource:Themes/Icons/Minimize%20Window.tticon")),
            //getResource<Draw::Path>(URL("resource:Themes/Icons/MultiColor.tticon")),
            [&]() { window->minimizeWindow(); }
        );
        window->addConstraint(minimizeWindowButton->box.outerTop() == box.top());
        window->addConstraint(minimizeWindowButton->box.outerRight() == maximizeWindowButton->box.outerLeft());
        window->addConstraint(minimizeWindowButton->box.outerBottom() == box.bottom);
    }
}

PipelineImage::Backing::ImagePixelMap WindowToolbarWidget::drawImage(std::shared_ptr<GUI::PipelineImage::Image> image)
{
    auto linearMap = Draw::PixelMap<wsRGBA>{ image->extent };
    fill(linearMap, wsRGBA{ 0x00000088 });

    return { std::move(image), std::move(linearMap) };
}

void WindowToolbarWidget::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, int &offset)
{
    required_assert(window);
    backingImage.loadOrDraw(*window, box.currentExtent(), [&](auto image) {
        return drawImage(image);
    }, "WindowToolbarWidget");

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

HitBox WindowToolbarWidget::hitBoxTest(glm::vec2 position) const
{
    if (trafficLightButtons->box.contains(position)) {
        return trafficLightButtons->hitBoxTest(position);
    }

    for (auto& widget : children) {
        if (widget->box.contains(position)) {
            return widget->hitBoxTest(position);
        }
    }
    return HitBox::MoveArea;
}

}
