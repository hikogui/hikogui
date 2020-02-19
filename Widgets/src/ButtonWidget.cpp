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
    Widget(), _label(std::move(label))
{
    box.leftMargin = 10.0;
    box.bottomMargin = 10.0;
    box.rightMargin = 10.0;
    box.topMargin = 10.0;
}

void ButtonWidget::update(
    bool modified,
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    ttauri_assert(window);

    if (modified) {
        // Draw something.
        let backgroundShape = glm::vec4{ 10.0, 10.0, -10.0, 0.0 };

        wsRGBA backgroundColor;
        wsRGBA labelColor;
        wsRGBA borderColor = wsRGBA{1.0, 1.0, 1.0, 1.0};
        if (value()) {
            backgroundColor = wsRGBA{ 0x4c4cffff };
            labelColor = wsRGBA{1.0, 1.0, 1.0, 1.0};
        } else {
            backgroundColor = wsRGBA{ 0x4c884cff };
            labelColor = wsRGBA{0.0, 0.0, 0.0, 1.0};
        }
        if (pressed()) {
            backgroundColor = wsRGBA{ 0x4c4cffff };
            labelColor = wsRGBA{0.0, 0.0, 0.0, 1.0};
        }

        auto buttonPath = Path();
        let rectangle = rect2{{0.5f, 0.5f}, { box.currentExtent().width() - 1.0f, box.currentExtent().height() - 1.0f }};
        buttonPath.addRectangle(rectangle, backgroundShape);

        drawing.clear();
        drawing.addPath(buttonPath, backgroundColor);
        drawing.addStroke(buttonPath, borderColor, 1.0);

        let labelStyle = TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 40.0, labelColor, 0.0, TextDecoration::None);

        labelShapedText = ShapedText(label(), labelStyle, box.currentExtent(), Alignment::MiddleCenter);

        window->device->SDFPipeline->prepareAtlas(labelShapedText);
    }

    backingImage.loadOrDraw(*window, box.currentExtent(), [&](auto image) {
        return drawImage(image);
    }, "Button", label(), value(), enabled(), focus(), pressed());

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

        backingImage.image->placeVertices(location, image_vertices);
    }

    window->device->SDFPipeline->placeVertices(labelShapedText, T2D(box.currentPosition()), box.currentRectangle(), depth, sdf_vertices);

    return Widget::update(modified, flat_vertices, image_vertices, sdf_vertices);
}

PipelineImage::Backing::ImagePixelMap ButtonWidget::drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept
{
    auto linearMap = PixelMap<wsRGBA>{image->extent};
    fill(linearMap);
    //composit(linearMap, drawing, window->subpixelOrientation);
    composit(linearMap, drawing, SubpixelOrientation::Unknown);

    return { std::move(image), std::move(linearMap) };
}

void ButtonWidget::handleMouseEvent(GUI::MouseEvent event) noexcept {
    if (enabled()) {
        window->setCursor(GUI::Cursor::Clickable);
        set_pressed(event.down.leftButton);

        if (event.type == GUI::MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            set_value(!value());
        }

    } else {
        window->setCursor(GUI::Cursor::Default);
    }
}

}
