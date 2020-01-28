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

void Widget::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, int &offset) noexcept
{
    for (auto &child : children) {
        child->pipelineImagePlaceVertices(vertices, offset);
    }
}

void Widget::pipelineFlatPlaceVertices(gsl::span<PipelineFlat::Vertex> &vertices, int &offset) noexcept
{
    for (auto &child : children) {
        child->pipelineFlatPlaceVertices(vertices, offset);
    }
}

void Widget::pipelineMSDFPlaceVertices(gsl::span<PipelineMSDF::Vertex> &vertices, int &offset) noexcept
{
    for (auto &child : children) {
        child->pipelineMSDFPlaceVertices(vertices, offset);
    }
}

HitBox Widget::hitBoxTest(glm::vec2 position) const noexcept
{
    for (auto& widget : children) {
        if (widget->box.contains(position)) {
            return widget->hitBoxTest(position);
        }
    }
    return HitBox::NoWhereInteresting;
}

void Widget::handleMouseEvent(MouseEvent const event) noexcept
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
