// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Widget.hpp"
#include "TTauri/GUI/utils.hpp"

namespace TTauri::GUI::Widgets {

Widget::Widget() noexcept
{
}

Device *Widget::device() const noexcept
{
    ttauri_assert(window);
    auto device = window->device;
    ttauri_assert(device);
    return device;
}

void Widget::setParent(Widget *parent) noexcept
{
    this->window = parent->window;
    this->parent = parent;
    this->depth = parent->depth - 0.001f;
}

bool Widget::updateAndPlaceVertices(
    bool,
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineBox::Vertex> &box_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    bool r = false;

    for (auto &child : children) {
        r |= child->_updateAndPlaceVertices(flat_vertices, box_vertices, image_vertices, sdf_vertices);
    }
    return r;
}

HitBox Widget::hitBoxTest(vec position) noexcept
{
    auto r = box.contains(position) ?
        HitBox{this, depth} :
        HitBox{};

    for (auto& widget : children) {
        r = std::max(r, widget->hitBoxTest(position));
    }
    return r;
}

bool Widget::handleMouseEvent(MouseEvent const &event) noexcept
{
    return false;
}

bool Widget::handleKeyboardEvent(KeyboardEvent const &event) noexcept
{
    bool continueRendering = false;

    for (auto &widget: children) {
        continueRendering |= widget->_handleKeyboardEvent(event);
    }
    return continueRendering;
}

}
