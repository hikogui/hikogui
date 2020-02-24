// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/LineInputWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace TTauri::Text;
using namespace std::literals;

LineInputWidget::LineInputWidget(std::string const label) noexcept :
    Widget(), label(std::move(label))
{
}

bool LineInputWidget::updateAndPlaceVertices(
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

    if (modified) {
        let labelStyle = TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 14.0, labelColor, 0.0, TextDecoration::None);

        labelShapedText = ShapedText(label, labelStyle, box.currentExtent(), Alignment::MiddleCenter);

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
        box.currentRectangle().expand(10.0)
    );

    window->device->SDFPipeline->placeVertices(sdf_vertices, labelShapedText, T2D(box.currentPosition()), box.currentRectangle(), depth);

    return Widget::updateAndPlaceVertices(modified, flat_vertices, box_vertices, image_vertices, sdf_vertices);
}

bool LineInputWidget::handleMouseEvent(GUI::MouseEvent event) noexcept {
    auto r = false;

    if (enabled) {
        window->setCursor(GUI::Cursor::Clickable);

        r |= assign_and_compare(pressed, static_cast<bool>(event.down.leftButton));

        if (event.type == GUI::MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            r |= assign_and_compare(value, !value);
        }

    } else {
        window->setCursor(GUI::Cursor::Default);
    }

    return r;
}

}
