//
//  Frame.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "View.hpp"
#include "Window.hpp"
#include <boost/assert.hpp>
#include <TTauri/utils.hpp>

namespace TTauri {
namespace GUI {

View::View()
{
}

void View::setParent(const std::shared_ptr<View> &parent)
{
    this->parent = parent;
    this->window = parent->window;
}

void View::setRectangle(glm::vec2 position, u16vec2 extent)
{
    this->position = position;
    this->extent = extent;
}

void View::add(std::shared_ptr<View> view)
{
    children.push_back(view);
    view->setParent(shared_from_this());
}


size_t View::backingPipelineRender(const gsl::span<BackingPipeline_vulkan::Vertex> &vertices, size_t offset)
{
    for (auto child : children) {
        offset = child->backingPipelineRender(vertices, offset);
    }
    return offset;
}


}}
