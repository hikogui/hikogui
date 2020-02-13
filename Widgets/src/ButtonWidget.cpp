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

void ButtonWidget::update(bool modified) noexcept
{
    if ((update_count++ % 60) == 0) {
        modified = true;
    }

    if (modified) {
        // Draw something.
        let backgroundShape = glm::vec4{ 10.0, 10.0, -10.0, 0.0 };

        wsRGBA backgroundColor;
        wsRGBA labelColor1;
        wsRGBA labelColor2;
        wsRGBA borderColor = wsRGBA{1.0, 1.0, 1.0, 1.0};
        if (value()) {
            backgroundColor = wsRGBA{ 0x4c4cffff };
            labelColor1 = wsRGBA{1.0, 1.0, 1.0, 1.0};
            labelColor2 = wsRGBA{0.0, 1.0, 1.0, 1.0};
        } else {
            backgroundColor = wsRGBA{ 0x4c884cff };
            labelColor1 = wsRGBA{0.0, 0.0, 0.0, 1.0};
            labelColor2 = wsRGBA{0.0, 0.0, 0.0, 1.0};
        }
        if (pressed()) {
            backgroundColor = wsRGBA{ 0x4c4cffff };
            labelColor1 = wsRGBA{0.0, 0.0, 0.0, 1.0};
            labelColor2 = wsRGBA{1.0, 0.0, 0.0, 1.0};
        }

    #pragma warning(suppress: 6001)
        let rectangle = rect2{{0.5f, 0.5f}, { box.currentExtent().width() - 1.0f, box.currentExtent().height() - 1.0f }};

        drawing.clear();

        auto buttonPath = Path();
        buttonPath.addRectangle(rectangle, backgroundShape);
        drawing.addPath(buttonPath, backgroundColor);
        drawing.addStroke(buttonPath, borderColor, 1.0);

        let labelStyle1 = TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 14.0, labelColor1, TextDecoration::None);
        let labelStyle2 = TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 14.0, labelColor2, TextDecoration::None);

        labelShapedText1 = ShapedText(label(), labelStyle1, Alignment::BaseCenter, box.currentExtent(), box.currentExtent());
        labelShapedText2 = ShapedText(label(), labelStyle2, Alignment::BaseCenter, box.currentExtent(), box.currentExtent());

        if ((update_count / 60) % 2 == 0) {
            drawing += labelShapedText2.get_path();
        }

        window->device->MSDFPipeline->prepareAtlas(labelShapedText1);
    }

    return Widget::update(modified);
}

void ButtonWidget::pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex>& vertices, ssize_t& offset) noexcept
{
    ttauri_assert(window);

    let blink = (update_count / 60) % 2 == 0;

    backingImage.loadOrDraw(*window, box.currentExtent(), [&](auto image) {
        return drawImage(image);
    }, "Button", label(), value(), enabled(), focus(), pressed(), blink);
 
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

void ButtonWidget::pipelineMSDFPlaceVertices(gsl::span<GUI::PipelineMSDF::Vertex>& vertices, ssize_t& offset) noexcept
{
    ttauri_assert(window);

    if ((update_count / 60) % 2 == 0) {
        window->device->MSDFPipeline->placeVertices(labelShapedText1, T2D(box.currentPosition()), box.currentRectangle(), depth, vertices, offset);
    }

    Widget::pipelineMSDFPlaceVertices(vertices, offset);
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
