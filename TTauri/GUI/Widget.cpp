// Copyright 2019 Pokitec
// All rights reserved.

#include "Widget.hpp"
#include "TTauri/GUI/all.hpp"
#include <boost/assert.hpp>
#include <TTauri/utils.hpp>

namespace TTauri::GUI {

Widget::Widget()
{
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


}
