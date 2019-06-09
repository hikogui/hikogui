// Copyright 2019 Pokitec
// All rights reserved.

#include "Widget.hpp"
#include "Window.hpp"
#include "TTauri/GUI/all.hpp"
#include <boost/assert.hpp>
#include <TTauri/utils.hpp>

namespace TTauri::GUI {

Widget::Widget()
{
}

std::shared_ptr<Device> Widget::device()
{
    return window.lock()->device.lock();
}

void Widget::setParent(const std::shared_ptr<Widget> &parent)
{
    this->window = parent->window;
    this->parent = move(parent);
}

void Widget::add(std::shared_ptr<Widget> widget)
{
    widget->setParent(shared_from_this());
    children.push_back(move(widget));
}

void Widget::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, size_t &offset)
{
    for (auto child : children) {
        child->pipelineImagePlaceVertices(vertices, offset);
    }
}

void Widget::handleMouseEvent(MouseEvent const event) {
    assert(event.type != MouseEvent::Type::None);

    let targetWidget = currentMouseTarget.lock();
    if (targetWidget) {
        if (targetWidget->box.contains(event.position)) {
            targetWidget->handleMouseEvent(event);
            if (event.type == MouseEvent::Type::Exited) {
                currentMouseTarget.reset();
            }

            // We completed sending the mouse event to the correct widget.
            return;

        } else {
            // We exited the previous target widget, send a exited event.
            targetWidget->handleMouseEvent(ExitedMouseEvent());
            currentMouseTarget.reset();
        }
    }

    if (event.type == MouseEvent::Type::Exited) {
        // We do not have any targets to send this event.
        return;
    }

    for (auto& widget : children) {
        if (widget->box.contains(event.position)) {
            currentMouseTarget = widget;
            return widget->handleMouseEvent(event);
        }
    }
    window.lock()->setCursor(Cursor::Default);
}

}
