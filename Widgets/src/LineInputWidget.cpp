// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "TTauri/Widgets/LineInputWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace TTauri::Text;
using namespace std::literals;

LineInputWidget::LineInputWidget(std::string const label, TextStyle style) noexcept :
    Widget(),
    label(std::move(label)),
    field(style),
    shapedText()
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
        backgroundColor = vec::color(0.1, 0.1, 0.1);
    } else {
        backgroundColor = vec::color(0.01, 0.01, 0.01);
    }

    if (hover || focus) {
        borderColor = vec::color(0.072, 0.072, 1.0);
    } else {
        borderColor = vec::color(0.1, 0.1, 0.1);
    }

    labelColor = vec{1.0, 1.0, 1.0, 1.0};

    auto textRectangle = expand(box.currentRectangle(), -5.0f);

    if (modified()) {
        field.setExtent(textRectangle.extent());
        std::tie(leftCaretPosition, rightCaretPosition) = field.carets();

        if (ssize(field) == 0) {
            let labelStyle = TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 14.0, labelColor, 0.0, TextDecoration::None);
            shapedText = ShapedText(label, labelStyle, textRectangle.extent(), Alignment::MiddleLeft);

        } else {
            shapedText = field.shapedText();
        }

        window->device->SDFPipeline->prepareAtlas(shapedText);
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

    let text_translate = mat::T(textRectangle.offset().z(elevation));

    window->device->SDFPipeline->placeVertices(
        sdf_vertices,
        shapedText,
        text_translate,
        box.currentRectangle()
    );

    if (leftCaretPosition.w() == 1.0) {
        let leftCaretBox = rect(leftCaretPosition, vec{1.0, 14.0});

        PipelineFlat::DeviceShared::placeVerticesBox(
            flat_vertices,
            text_translate * leftCaretBox,
            vec::color(1.0, 0.5, 0.5),
            box.currentRectangle(),
            elevation + 0.0005f
        );
    }

    continueRendering |= Widget::updateAndPlaceVertices(flat_vertices, box_vertices, image_vertices, sdf_vertices);
    return continueRendering;
}


bool LineInputWidget::handleCommand(string_ltag command) noexcept
{
    LOG_DEBUG("LineInputWidget: Received command: {}", tt5_decode(command));
    if (!enabled) {
        return false;
    }

    // This lock is held during rendering, only update the field when holding this lock.
    std::scoped_lock lock(GUI_globals->mutex);

    auto continueRendering = false;

    continueRendering |= field.handleCommand(command);
    

    return continueRendering;
}

bool LineInputWidget::handleKeyboardEvent(GUI::KeyboardEvent const &event) noexcept
{
    auto continueRendering = Widget::handleKeyboardEvent(event);

    if (!enabled) {
        return false;
    }

    // This lock is held during rendering, only update the field when holding this lock.
    std::scoped_lock lock(GUI_globals->mutex);

    switch (event.type) {
    case GUI::KeyboardEvent::Type::Grapheme:
        return continueRendering | field.insertGrapheme(event.grapheme);

    case GUI::KeyboardEvent::Type::PartialGrapheme:
        return continueRendering | field.insertPartialGrapheme(event.grapheme);

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
