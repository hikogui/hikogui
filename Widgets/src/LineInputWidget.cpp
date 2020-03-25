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
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineBox::Vertex> &box_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    ttauri_assert(window);

    auto continueRendering = false;

    // Draw something.
    let cornerShapes = vec{0.0, 0.0, 0.0, 0.0};

    vec backgroundColor;
    vec labelColor;
    vec borderColor;

    if (hover || focus) {
        backgroundColor = vec::color(0.3, 0.3, 0.3);
    } else {
        backgroundColor = vec::color(0.1, 0.1, 0.1);
    }

    if (hover || focus) {
        borderColor = vec::color(0.072, 0.072, 1.0);
    } else {
        borderColor = vec::color(0.3, 0.3, 0.3);
    }

    labelColor = vec{1.0, 1.0, 1.0, 1.0};

    auto textRectangle = expand(box.currentRectangle(), -5.0f);

    if (modified()) {
        let labelStyle = TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 14.0, labelColor, 0.0, TextDecoration::None);

        labelShapedText = ShapedText(label, labelStyle, textRectangle.extent(), Alignment::MiddleLeft);

        window->device->SDFPipeline->prepareAtlas(labelShapedText);
    }

    PipelineBox::DeviceShared::placeVertices(
        box_vertices,
        elevation,
        box.currentRectangle(),
        backgroundColor,
        1.0f,
        borderColor,
        0.0f,
        cornerShapes,
        expand(box.currentRectangle(), 10.0)
    );

    window->device->SDFPipeline->placeVertices(
        sdf_vertices,
        labelShapedText,
        mat::T(textRectangle.offset().z(elevation)),
        box.currentRectangle()
    );

    continueRendering |= Widget::updateAndPlaceVertices(flat_vertices, box_vertices, image_vertices, sdf_vertices);
    return continueRendering;
}


bool LineInputWidget::handleCommand(string_ltag command) noexcept
{
    LOG_DEBUG("LineInputWidget: Received command: {}", tt5_decode(command));
    if (!enabled) {
        return false;
    }

    if (command == "text."_ltag) {

    }

    return false;
}

bool LineInputWidget::handleKeyboardEvent(GUI::KeyboardEvent const &event) noexcept
{
    auto continueRendering = Widget::handleKeyboardEvent(event);

    if (!enabled) {
        return false;
    }

    switch (event.type) {
    case GUI::KeyboardEvent::Type::Grapheme:
        //text.insertGrapheme(grapheme);
        return true;

    case GUI::KeyboardEvent::Type::PartialGrapheme:
        //text.insertPartialGrapheme(event.grapheme);
        return true;

    default:;
    }

    return continueRendering;
}

bool LineInputWidget::handleMouseEvent(GUI::MouseEvent const &event) noexcept {
    auto continueRendering = Widget::handleMouseEvent(event);

    if (!enabled) {
        return false;
    }

    return continueRendering;
}

HitBox LineInputWidget::hitBoxTest(vec position) noexcept
{
    if (box.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::TextEdit : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
