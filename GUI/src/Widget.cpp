// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Widget.hpp"
#include "TTauri/GUI/utils.hpp"

namespace TTauri::GUI::Widgets {

Widget::Widget(Window &window, Widget *parent) noexcept :
    window(window), parent(parent), renderTrigger(&window.renderTrigger), elevation(parent ? parent->elevation + 0.001f : 0.0f) {}

Device *Widget::device() const noexcept
{
    auto device = window.device;
    ttauri_assert(device);
    return device;
}

void Widget::updateAndPlaceVertices(
    cpu_utc_clock::time_point displayTimePoint,
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineBox::Vertex> &box_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    for (auto &child : children) {
        child->updateAndPlaceVertices(displayTimePoint, flat_vertices, box_vertices, image_vertices, sdf_vertices);
    }
}

void Widget::handleCommand(string_ltag command) noexcept {
    if (command == "gui.widget.next"_ltag) {
        window.updateToNextKeyboardTarget(this);
    } else if (command == "gui.widget.prev"_ltag) {
        window.updateToPrevKeyboardTarget(this);
    }
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
