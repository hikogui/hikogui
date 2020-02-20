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
    vspan<PipelineBox::Vertex> &box_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    ttauri_assert(window);

    // Draw something.
    let cornerShapes = glm::vec4{ 10.0, 10.0, -10.0, 0.0 };

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

    if (modified) {
        let labelStyle = TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 32.0, labelColor, 0.0, TextDecoration::None);

        labelShapedText = ShapedText(label(), labelStyle, box.currentExtent(), Alignment::MiddleCenter);

        window->device->SDFPipeline->prepareAtlas(labelShapedText);
    }

    PipelineBox::DeviceShared::placeVertices(
        box_vertices,
        depth,
        box.currentRectangle(),
        R16G16B16A16SFloat{backgroundColor},
        1.0f,
        R16G16B16A16SFloat{borderColor},
        6.0f,
        R16G16B16A16SFloat{cornerShapes},
        box.currentOuterRectangle()
    );

    window->device->SDFPipeline->placeVertices(sdf_vertices, labelShapedText, T2D(box.currentPosition()), box.currentRectangle(), depth);

    return Widget::update(modified, flat_vertices, box_vertices, image_vertices, sdf_vertices);
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
