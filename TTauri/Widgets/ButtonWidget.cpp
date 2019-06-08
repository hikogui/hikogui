// Copyright 2019 Pokitec
// All rights reserved.

#include "ButtonWidget.hpp"
#include "TTauri/GUI/all.hpp"
#include "TTauri/Draw/all.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>

namespace TTauri::Widgets {

ButtonWidget::ButtonWidget(std::string const label) :
    label(std::move(label)), Widget()
{
}



void ButtonWidget::pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex>& vertices, size_t& offset)
{
    auto vulkanDevice = device();

    if (!window.lock()->resizing) {
        currentExtent = box.currentExtent();
    }
    let currentScale = box.currentExtent() / currentExtent;

    let key = (boost::format("ButtonWidget(%i,%i,%s,%i)") % currentExtent.x % currentExtent.y % label % static_cast<size_t>(state)).str();

    auto &imagePtrRef = imagePerState.at(static_cast<size_t>(state));
    vulkanDevice->imagePipeline->exchangeImage(imagePtrRef, key, currentExtent);

    auto &image = *imagePtrRef;
    drawImage(image, State::ENABLED);

    GUI::PipelineImage::ImageLocation location;
    location.depth = depth + 0.0;
    location.origin = {0.0, 0.0};
    location.position = box.currentPosition() + location.origin;
    location.scale = currentScale;
    location.rotation = 0.0;
    location.alpha = 1.0;
    location.clippingRectangle = box.currentRectangle();

    image.placeVertices(location, vertices, offset);
}

void ButtonWidget::drawImage(GUI::PipelineImage::Image &image, State state)
{
    if (image.drawn) {
        return;
    }

    auto vulkanDevice = device();

    auto linearMap = Draw::PixelMap<uint64_t>{image.extent};
    linearMap.clear();

    // Draw something.
    let backgroundShape = glm::vec4{ 10.0, 10.0, -10.0, 0.0 };
    let &labelFont = Draw::fonts->get("Themes/Fonts/Roboto/Roboto-Regular.ttf");
    let labelFontSize = 12.0;

    Color_sRGBLinear backgroundColor{};
    Color_sRGBLinear labelColor{};
    if (label == "Foo Bar") {
        backgroundColor = color_cast<Color_sRGBLinear>(Color_sRGB{ glm::vec4{0.0f, 0.0f, 0.0f, 1.0f} });
        labelColor = color_cast<Color_sRGBLinear>(Color_sRGB{ glm::vec4{1.0f, 1.0f, 1.0f, 1.0f} });
    } else {
        backgroundColor = color_cast<Color_sRGBLinear>(Color_sRGB{ glm::vec4{1.0f, 1.0f, 1.0f, 1.0f} });
        labelColor = color_cast<Color_sRGBLinear>(Color_sRGB{ glm::vec4{0.0f, 0.0f, 0.0f, 1.0f} });
    }

    let rect = rect2{{0.0f, 0.0f}, { static_cast<float>(image.extent.width()), static_cast<float>(image.extent.height()) }};
    let fontCenter = labelFontSize * 0.5f;
    let labelLocation = midpoint(rect) + glm::vec2(0.0f, -fontCenter);

    auto buttonBackgroundMask = Draw::Path();
    buttonBackgroundMask.addRectangle(rect, backgroundShape);
    buttonBackgroundMask.render(linearMap, backgroundColor, Draw::SubpixelMask::Orientation::RedLeft);

    auto textMask = Draw::Path();
    textMask.addText(label, labelFont, labelLocation, labelFontSize, 0.0f, Draw::HorizontalAlignment::Center);
    textMask.render(linearMap, labelColor, Draw::SubpixelMask::Orientation::RedLeft);

    auto pixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(image.extent);
    copyLinearToGamma(pixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(image);
    image.drawn = true;
}


}
