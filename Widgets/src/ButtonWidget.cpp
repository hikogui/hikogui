// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ButtonWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace TTauri::Text;
using namespace std::literals;

ButtonWidget::ButtonWidget(std::string const label) noexcept :
    Widget(), label(std::move(label))
{
    box.leftMargin = 10.0;
    box.bottomMargin = 10.0;
    box.rightMargin = 10.0;
    box.topMargin = 10.0;
}

void ButtonWidget::pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex>& vertices, int& offset) noexcept
{
    ttauri_assert(window);

    backingImage.loadOrDraw(*window, box.currentExtent(), [&](auto image) {
        return drawImage(image);
    }, "Button", label, state());
 
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

PipelineImage::Backing::ImagePixelMap ButtonWidget::drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept
{
    auto linearMap = PixelMap<wsRGBA>{image->extent};
    fill(linearMap);

    // Draw something.
    let backgroundShape = glm::vec4{ 10.0, 10.0, -10.0, 0.0 };
    let labelFontSize = 12.0;

    wsRGBA backgroundColor;
    wsRGBA labelColor;
    wsRGBA borderColor = wsRGBA{1.0, 1.0, 1.0, 1.0};
    if (value) {
        backgroundColor = wsRGBA{ 0x4c4cffff };
        labelColor = wsRGBA{1.0, 1.0, 1.0, 1.0};
    } else {
        backgroundColor = wsRGBA{ 0x4c884cff };
        labelColor = wsRGBA{0.0, 0.0, 0.0, 1.0};
    }
    if (pressed) {
        backgroundColor = wsRGBA{ 0x4c4cffff };
        labelColor = wsRGBA{0.0, 0.0, 0.0, 1.0};
    }

#pragma warning(suppress: 6001)
    let rectangle = rect2{{0.5f, 0.5f}, { static_cast<float>(image->extent.width()) - 1.0f, static_cast<float>(image->extent.height()) - 1.0f }};

    auto drawing = Path();

    auto buttonPath = Path();
    buttonPath.addRectangle(rectangle, backgroundShape);
    drawing.addPath(buttonPath, backgroundColor);
    drawing.addStroke(buttonPath, borderColor, 1.0);

    let labelStyle = TextStyle("Arial", FontVariant{FontWeight::Regular, false}, labelFontSize, labelColor, TextDecoration::None);
    let labelShapedText = ShapedText(label, labelStyle, Alignment::MiddleCenter, rectangle.extent, rectangle.extent);
    drawing += labelShapedText.toPath();

    composit(linearMap, drawing, window->subpixelOrientation);

    return { std::move(image), std::move(linearMap) };
}

void ButtonWidget::handleMouseEvent(GUI::MouseEvent event) noexcept {
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
