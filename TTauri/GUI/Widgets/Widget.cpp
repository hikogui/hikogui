// Copyright 2019 Pokitec
// All rights reserved.

#include "Widget.hpp"
#include "utils.hpp"
#include <boost/assert.hpp>
#include <TTauri/utils.hpp>

namespace TTauri::GUI::Widgets {

Widget::Widget()
{
}

Device *Widget::device() const
{
    required_assert(window);
    auto device = window->device;
    required_assert(device);
    return device;
}

void Widget::setParent(Widget *parent)
{
    this->window = parent->window;
    this->parent = parent;
}

void Widget::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, int &offset)
{
    for (auto &child : children) {
        child->pipelineImagePlaceVertices(vertices, offset);
    }
}

HitBox Widget::hitBoxTest(glm::vec2 position) const
{
    for (auto& widget : children) {
        if (widget->box.contains(position)) {
            return widget->hitBoxTest(position);
        }
    }
    return HitBox::NoWhereInteresting;
}

void Widget::handleMouseEvent(MouseEvent const event)
{
    assert(event.type != MouseEvent::Type::None);

    let targetWidget = currentMouseTarget;
    if (targetWidget) {
        if (targetWidget->box.contains(event.position)) {
            targetWidget->handleMouseEvent(event);
            if (event.type == MouseEvent::Type::Exited) {
                currentMouseTarget = nullptr;
            }

            // We completed sending the mouse event to the correct widget.
            return;

        } else {
            // We exited the previous target widget, send a exited event.
            targetWidget->handleMouseEvent(ExitedMouseEvent(event.position));
            currentMouseTarget = nullptr;
        }
    }

    if (event.type == MouseEvent::Type::Exited) {
        // We do not have any targets to send this event.
        return;
    }

    for (auto& widget : children) {
        if (widget->box.contains(event.position)) {
            currentMouseTarget = widget.get();
            return widget->handleMouseEvent(event);
        }
    }
    window->setCursor(Cursor::Default);
}

}
