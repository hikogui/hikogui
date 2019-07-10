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

    trafficLightButtons = addWidget<WindowTrafficLightsWidget>();
    window->addConstraint(trafficLightButtons->box.outerTop() == box.top());
    window->addConstraint(trafficLightButtons->box.outerLeft() == box.left);
    window->addConstraint(trafficLightButtons->box.outerBottom() == box.bottom);

    closeWindowButton = addWidget<ToolbarButtonWidget>(
        0.3f * getResource<Draw::Path>(URL("resource:Themes/Icons/Close%20Window.tticon")),
        [&]() { window->closeWindow(); }
    );
    closeWindowButton->hoverBackgroundColor = { 0xdd0000ff };
    closeWindowButton->pressedBackgroundColor = { 0xff0000ff };
    window->addConstraint(closeWindowButton->box.outerTop() == box.top());
    window->addConstraint(closeWindowButton->box.outerRight() == box.right());
    window->addConstraint(closeWindowButton->box.outerBottom() == box.bottom);

    maximizeWindowButton = addWidget<ToolbarButtonWidget>(
        0.3f * getResource<Draw::Path>(URL("resource:Themes/Icons/Maximize%20Window.tticon")),
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
        //getResource<Draw::Path>(URL("resource:Themes/Icons/Minimize%20Window.tticon")),
        getResource<Draw::Path>(URL("resource:Themes/Icons/MultiColor.tticon")),
        [&]() { window->minimizeWindow(); }
    );
    window->addConstraint(minimizeWindowButton->box.outerTop() == box.top());
    window->addConstraint(minimizeWindowButton->box.outerRight() == maximizeWindowButton->box.outerLeft());
    window->addConstraint(minimizeWindowButton->box.outerBottom() == box.bottom);

}

void WindowToolbarWidget::drawBackingImage()
{
    if (backingImage->drawn) {
        return;
    }

    auto vulkanDevice = device();

    auto linearMap = Draw::PixelMap<wsRGBA>{ backingImage->extent };
    fill(linearMap, wsRGBA{ 0x00000088 });

    auto fullPixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(backingImage->extent);
    fill(fullPixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(*backingImage);
    backingImage->drawn = true;
}

void WindowToolbarWidget::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, size_t &offset)
{
    let rectangle = box.currentRectangle();
    required_assert(rectangle.extent.width() > 0 && rectangle.extent.height() > 0);

    let key = BinaryKey("WindowToolbarWidget", rectangle.extent);

    auto vulkanDevice = device();

    // backingImage keeps track of use count.
    vulkanDevice->imagePipeline->exchangeImage(backingImage, key, rectangle.extent);
    drawBackingImage();

    GUI::PipelineImage::ImageLocation location;
    location.depth = depth + 0.0f;
    location.origin = {0.0, 0.0};
    location.position = rectangle.offset + location.origin;
    location.rotation = 0.0;
    location.scale = {1.0, 1.0};
    location.alpha = 1.0;
    location.clippingRectangle = rectangle;

    backingImage->placeVertices(location, vertices, offset);

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
