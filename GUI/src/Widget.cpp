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
    this->elevation = parent->elevation + 0.001f;
}

bool Widget::updateAndPlaceVertices(
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineBox::Vertex> &box_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    bool r = false;

    for (auto &child : children) {
        r |= child->updateAndPlaceVertices(flat_vertices, box_vertices, image_vertices, sdf_vertices);
    }
    return r;
}

HitBox Widget::hitBoxTest(vec position) noexcept
{
    auto r = box.contains(position) ?
        HitBox{this, elevation} :
        HitBox{};

    for (auto& widget : children) {
        r = std::max(r, widget->hitBoxTest(position));
    }
    return r;
}

}
