// Copyright 2019 Pokitec
// All rights reserved.

#include "ButtonWidget.hpp"
#include "utils.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>
#include <typeinfo>

namespace TTauri::Widgets {

using namespace std::literals;

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

    key.update("Button", currentExtent, label, state());

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

void ButtonWidget::drawImage(GUI::PipelineImage::Image &image)
{
    if (image.drawn) {
        return;
    }

    auto vulkanDevice = device();

    auto linearMap = Draw::PixelMap<wsRGBA>{image.extent};
    clear(linearMap);

    // Draw something.
    let backgroundShape = glm::vec4{ 10.0, 10.0, -10.0, 0.0 };
    let &labelFont = Draw::fonts->get("Themes/Fonts/Roboto/Roboto-Regular.ttf");
    let labelFontSize = 12.0;

    wsRGBA backgroundColor;
    wsRGBA labelColor;
    wsRGBA borderColor = {1.0, 1.0, 1.0, 1.0};
    if (value) {
        backgroundColor = { 0x4c4cffff };
        labelColor = {1.0, 1.0, 1.0, 1.0};
    } else {
        backgroundColor = { 0x4c884cff };
        labelColor = {0.0, 0.0, 0.0, 1.0};
    }
    if (pressed) {
        backgroundColor = { 0x4c4cffff };
        labelColor = {0.0, 0.0, 0.0, 1.0};
    }

#pragma warning(suppress: 6001)
    let rectangle = rect2{{1.0f, 1.0f}, { static_cast<float>(image.extent.width()) - 2.0f, static_cast<float>(image.extent.height()) - 2.0f }};
    let labelLocation = midpoint(rectangle);
    //let labelLocation = glm::vec2{0.0, 0.0};

    auto drawing = Draw::Drawing();

    auto buttonPath = Draw::Path();
    buttonPath.addRectangle(rectangle, backgroundShape);
    drawing.addPath(buttonPath, backgroundColor);
    drawing.addStroke(buttonPath, borderColor, 2.0);

    auto textPath = Draw::Path();

    let labelGlyphs = T2D(labelLocation, labelFontSize) * labelFont.getGlyphs(label);
    textPath.addText(labelGlyphs, Draw::Alignment::MiddleCenter);
    drawing.addPath(textPath, labelColor);

    draw(linearMap, drawing, Draw::SubpixelOrientation::RedLeft);

    auto pixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(image.extent);
    copyLinearToGamma(pixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(image);
    image.drawn = true;
}

void ButtonWidget::handleMouseEvent(GUI::MouseEvent event) {
    if (enabled) {
        window.lock()->setCursor(GUI::Cursor::Clickable);
        pressed = event.down.leftButton;

        if (event.type == GUI::MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            value = !value;
        }

    } else {
        window.lock()->setCursor(GUI::Cursor::Default);
    }
}

}
