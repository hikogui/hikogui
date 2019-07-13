// Copyright 2019 Pokitec
// All rights reserved.

#include "ButtonWidget.hpp"
#include "utils.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace std::literals;

ButtonWidget::ButtonWidget(std::string const label) :
    label(std::move(label)), Widget()
{
    box.leftMargin = 10.0;
    box.bottomMargin = 10.0;
    box.rightMargin = 10.0;
    box.topMargin = 10.0;
}

void ButtonWidget::pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex>& vertices, size_t& offset)
{
    if (pixelMapFuture && pixelMapFuture->valid()) {
        auto [newImage, newPixelMap] = pixelMapFuture->get();

        if (newPixelMap) {
            auto stagingMap = device()->imagePipeline->getStagingPixelMap(newImage->extent);
            fill(stagingMap, newPixelMap);
            device()->imagePipeline->updateAtlasWithStagingPixelMap(*newImage);
            newImage->state = GUI::PipelineImage::Image::State::Uploaded;
        }

        if (newImage->state == GUI::PipelineImage::Image::State::Uploaded) {
            image = newImage;
            pixelMapFuture = {};
        }
    }

    if (!window->resizing) {
        currentExtent = box.currentExtent();

        pickle(key, "Button", currentExtent, label, state());

        if ((image == nullptr || image->key != key) && !pixelMapFuture) {
            auto newImage = device()->imagePipeline->getImage(key, currentExtent);

            switch (newImage->state) {
            case GUI::PipelineImage::Image::State::Uploaded:
                image = newImage;
                break;

            case GUI::PipelineImage::Image::State::Drawing: {
                auto p = std::promise<ImagePixelMap>();
                pixelMapFuture = p.get_future();
                p.set_value({image, Draw::PixelMap<wsRGBA>{}});
                } break;

            case GUI::PipelineImage::Image::State::Uninitialized:
                // Try and draw the image, multiple calls will be dropped by the callee.
                pixelMapFuture = std::async([=]() {
                    return drawImage(newImage);
                });
                break;
            }
        }
    }

    if (image == nullptr) {
        return;
    }

    let currentScale = box.currentExtent() / extent2{image->extent};

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

ButtonWidget::ImagePixelMap ButtonWidget::drawImage(std::shared_ptr<GUI::PipelineImage::Image> image)
{
    auto expected = GUI::PipelineImage::Image::State::Uninitialized;
    if (!image->state.compare_exchange_strong(expected, GUI::PipelineImage::Image::State::Drawing)) {
        // Another thread has started drawing.
        return {image, Draw::PixelMap<wsRGBA>{}};
    }

    auto linearMap = Draw::PixelMap<wsRGBA>{image->extent};
    fill(linearMap);

    // Draw something.
    let backgroundShape = glm::vec4{ 10.0, 10.0, -10.0, 0.0 };
    let &labelFont = getResource<Draw::Font>(URL("resource:Themes/Fonts/Roboto/Roboto-Regular.ttf"));
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
    let rectangle = rect2{{0.5f, 0.5f}, { static_cast<float>(image->extent.width()) - 1.0f, static_cast<float>(image->extent.height()) - 1.0f }};
    let labelLocation = midpoint(rectangle);
    //let labelLocation = glm::vec2{0.0, 0.0};

    auto drawing = Draw::Path();

    auto buttonPath = Draw::Path();
    buttonPath.addRectangle(rectangle, backgroundShape);
    drawing.addPath(buttonPath, backgroundColor);
    drawing.addStroke(buttonPath, borderColor, 1.0);

    let labelGlyphs = Draw::Alignment::MiddleCenter + T2D(labelLocation, labelFontSize) * labelFont.getGlyphs(label);
    drawing += labelGlyphs.toPath(labelColor);

    composit(linearMap, drawing, window->subpixelOrientation);

    return { std::move(image), std::move(linearMap) };
}

void ButtonWidget::handleMouseEvent(GUI::MouseEvent event) {
    if (enabled) {
        window->setCursor(GUI::Cursor::Clickable);
        pressed = event.down.leftButton;

        if (event.type == GUI::MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            value = !value;
        }

    } else {
        window->setCursor(GUI::Cursor::Default);
    }
}

}
