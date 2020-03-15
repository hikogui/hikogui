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

HitBox Widget::hitBoxTest(vec position) const noexcept
{
    for (auto& widget : children) {
        if (widget->box.contains(position)) {
            return widget->hitBoxTest(position);
        }
    }
    return HitBox::NoWhereInteresting;
}

bool Widget::handleMouseEvent(MouseEvent const &event) noexcept
{
    bool r = false;

    assert(event.type != MouseEvent::Type::None);

    let targetWidget = currentMouseTarget;
    if (targetWidget) {
        if (targetWidget->box.contains(event.position)) {
            r |= targetWidget->_handleMouseEvent(event);
            if (event.type == MouseEvent::Type::Exited) {
                currentMouseTarget = nullptr;
            }

            // We completed sending the mouse event to the correct widget.
            return r;

        } else {
            // We exited the previous target widget, send a exited event.
            r |= targetWidget->_handleMouseEvent(ExitedMouseEvent(event.position));
            currentMouseTarget = nullptr;
        }
    }

    if (event.type == MouseEvent::Type::Exited) {
        // We do not have any targets to send this event.
        return r;
    }

    for (auto& widget : children) {
        if (widget->box.contains(event.position)) {
            currentMouseTarget = widget.get();
            return r | widget->_handleMouseEvent(event);
        }
    }
    window->setCursor(Cursor::Default);

    return r;
}

bool Widget::handleKeyboardEvent(KeyboardEvent const &event) noexcept
{
    bool r = false;

    for (auto &widget: children) {
        r |= widget->_handleKeyboardEvent(event);
    }
    return r;
}

}
