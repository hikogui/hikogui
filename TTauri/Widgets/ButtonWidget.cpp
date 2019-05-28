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

    // Draw something.
    let backgroundColor = color_cast<Color_sRGBLinear>(Color_sRGB{ glm::vec4{0.2f, 0.2f, 0.2f, 1.0f} });
    let backgroundShape = glm::vec4{ 10.0, 10.0, -10.0, 0.0 };
    let labelFont = get_singleton<Draw::Fonts>()->get("Themes/Fonts/Roboto/Roboto-Regular.ttf");
    let labelColor = color_cast<Color_sRGBLinear>(Color_sRGB{ glm::vec4{1.0f, 1.0f, 1.0f, 1.0f} });
    let labelFontSize = 12.0;

    let rect = rect2{{0.0f, 0.0f}, { static_cast<float>(image.extent.width()), static_cast<float>(image.extent.height()) }};
    let fontCenter = labelFontSize * 0.5f;
    let labelLocation = midpoint(rect) + glm::vec2(0.0f, -fontCenter);

    auto buttonBackgroundMask = Draw::Path();
    buttonBackgroundMask.addRectangle(rect, backgroundShape);
    buttonBackgroundMask.render(pixelMap, backgroundColor, Draw::SubpixelMask::Orientation::RedLeft);

    auto textMask = Draw::Path();
    textMask.addText(label, labelFont, labelLocation, labelFontSize, 0.0f, Draw::HorizontalAlignment::Center);
    textMask.render(pixelMap, labelColor, Draw::SubpixelMask::Orientation::RedLeft);

    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(image);
    image.drawn = true;
}


}
