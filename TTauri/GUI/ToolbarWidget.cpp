// Copyright 2019 Pokitec
// All rights reserved.

#include "ToolbarWidget.hpp"
#include "WindowDecorationWidget.hpp"
#include "WindowTrafficLightsWidget.hpp"
#include "PipelineImage_Image.hpp"
#include "PipelineImage_Vertex.hpp"
#include "Device.hpp"
#include "TTauri/Draw/PixelMap.inl"
#include "TTauri/Draw/attributes.hpp"
#include "TTauri/Color.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>

namespace TTauri::GUI {

using namespace std;

ToolbarWidget::ToolbarWidget() :
    Widget()
{
}

void ToolbarWidget::setParent(Widget *parent)
{
    Widget::setParent(parent);

    auto _leftDecorationWidget = make_shared<WindowTrafficLightsWidget>();
    add(_leftDecorationWidget);
    leftDecorationWidget = _leftDecorationWidget.get();

    auto _rightDecorationWidget = make_shared<WindowDecorationWidget>(Draw::Alignment::TopRight);
    add(_rightDecorationWidget);
    rightDecorationWidget = _rightDecorationWidget.get();

    this->window->addConstraint(box.outerLeft() == parent->box.left);
    this->window->addConstraint(box.outerRight() == parent->box.right());
    this->window->addConstraint(box.outerTop() == parent->box.top());
    this->window->addConstraint(box.outerBottom() >= parent->box.bottom);
}


void ToolbarWidget::drawBackingImage()
{
    if (backingImage->drawn) {
        return;
    }

    auto vulkanDevice = device();

    auto linearMap = Draw::PixelMap<wsRGBA>{ backingImage->extent };
    fill(linearMap, wsRGBA{ 0x00000000 });

    auto fullPixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(backingImage->extent);
    fill(fullPixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(*backingImage);
    backingImage->drawn = true;
}

void ToolbarWidget::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, size_t &offset)
{
    let key = BinaryKey("ToolbarWidget", box.currentExtent());

    auto vulkanDevice = device();

    // backingImage keeps track of use count.
    vulkanDevice->imagePipeline->exchangeImage(backingImage, key, box.currentExtent());
    drawBackingImage();

    GUI::PipelineImage::ImageLocation location;
    location.depth = depth + 0.0f;
    location.origin = {0.0, 0.0};
    location.position = box.currentPosition() + location.origin;
    location.rotation = 0.0;
    location.scale = {1.0, 1.0};
    location.alpha = 1.0;
    location.clippingRectangle = box.currentRectangle();

    backingImage->placeVertices(location, vertices, offset);

    Widget::pipelineImagePlaceVertices(vertices, offset);
}

HitBox ToolbarWidget::hitBoxTest(glm::vec2 position) const
{
    for (auto& widget : children) {
        if (widget->box.contains(position)) {
            return widget->hitBoxTest(position);
        }
    }
    return HitBox::MoveArea;
}

}
