// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowToolbarWidget.hpp"
#include "WindowTrafficLightsWidget.hpp"
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

    
}

void WindowToolbarWidget::drawBackingImage()
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
    for (auto& widget : children) {
        if (widget->box.contains(position)) {
            return widget->hitBoxTest(position);
        }
    }
    return HitBox::MoveArea;
}

}
