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
    auto vulkanDevice = device<GUI::Device_vulkan>();
    let key = (boost::format("ButtonWidget(%i,%i,%i)") % extent.x % extent.y % static_cast<size_t>(state)).str();

    auto &imagePtrRef = imagePerState.at(static_cast<size_t>(state));
    vulkanDevice->imagePipeline->exchangeImage(imagePtrRef, key, extent);

    auto &image = *imagePtrRef;
    drawImage(image, State::ENABLED);

    GUI::PipelineImage::ImageLocation location;
    location.position = position;
    location.depth = depth + 0.0;
    location.origin = { image.extent.x * 0.5, image.extent.y * 0.5 };
    location.position = position + location.origin;
    location.rotation = 0.0;
    location.alpha = 1.0;
    location.clippingRectangle = {
        {position.x, position.y},
        extent
    };

    image.placeVertices(location, vertices, offset);
}

void ButtonWidget::drawImage(GUI::PipelineImage::Image &image, State state)
{
    if (image.drawn) {
        return;
    }

    auto vulkanDevice = device<GUI::Device_vulkan>();

    auto pixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(image.extent);
    pixelMap.fill({ {0.0f, 0.0f, 0.0f, 1.0f} });

    let myFont = get_singleton<Draw::Fonts>()->get("Themes/Fonts/Roboto/Roboto-Regular.ttf");
    let glyphIndex = myFont.characterMap.at('g');
    let glyph = myFont.glyphs.at(glyphIndex);

    // Draw something.
    let backgroundColor = color_cast<Color_sRGBLinear>(Color_sRGB{ glm::vec4{0.5f, 0.5f, 0.5f, 1.0f} });
    float topLeftCornerRadius = 10.0;
    float topRightCornerRadius = 10.0;
    float bottomLeftCornerRadius = 10.0;
    float bottomRightCornerRadius = 10.0;

    bool topLeftCornerIsRounded = true;
    bool topRightCornerIsRounded = true;
    bool bottomLeftCornerIsRounded = true;
    bool bottomRightCornerIsRounded = false;

    auto buttonShape = Draw::Path();
    buttonShape.moveTo({ 0.0, image.extent.y - topLeftCornerRadius });
    if (topLeftCornerIsRounded) {
        buttonShape.arcTo({topLeftCornerRadius, image.extent.y - topLeftCornerRadius}, { topLeftCornerRadius, image.extent.y });
    } else {
        buttonShape.lineTo({ topLeftCornerRadius, image.extent.y });
    }

    buttonShape.lineTo({ image.extent.x - topRightCornerRadius, image.extent.y });
    if (topRightCornerIsRounded) {
        buttonShape.arcTo({image.extent.x - topRightCornerRadius, image.extent.y - topRightCornerRadius}, { image.extent.x, image.extent.y - topRightCornerRadius });
    } else {
        buttonShape.lineTo({ image.extent.x, image.extent.y - topRightCornerRadius });
    }

    buttonShape.lineTo({ image.extent.x, bottomRightCornerRadius });
    if (bottomRightCornerIsRounded) {
        buttonShape.arcTo({image.extent.x - bottomRightCornerRadius, bottomRightCornerRadius}, { image.extent.x - bottomRightCornerRadius, 0.0 });
    } else {
        buttonShape.lineTo({ image.extent.x - bottomRightCornerRadius, 0.0 });
    }

    buttonShape.lineTo({ bottomLeftCornerRadius, 0.0 });
    if (bottomLeftCornerIsRounded) {
        buttonShape.arcTo({bottomLeftCornerRadius, bottomLeftCornerRadius}, { 0.0, bottomLeftCornerRadius });
    } else {
        buttonShape.lineTo({ 0.0, bottomLeftCornerRadius });
    }

    buttonShape.close();
    buttonShape.render(pixelMap, backgroundColor, Draw::SubpixelMask::Orientation::RedLeft);

    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(image);
    image.drawn = true;
}


}
