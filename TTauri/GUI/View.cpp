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

View::~View()
{
}

template <typename T>
std::shared_ptr<T> View::device()
{
    return lock_dynamic_cast<T>(window.lock()->device);
}

void View::setParent(const std::shared_ptr<View> &parent)
{
    this->parent = parent;
    this->window = parent->window;
}

void View::setRectangle(glm::vec3 position, glm::vec3 extent)
{
    this->position = position;
    this->extent = extent;
}

void View::add(std::shared_ptr<View> view)
{
    children.push_back(view);
    view->setParent(shared_from_this());
}


size_t View::BackingPipelineRender(BackingPipeline::Vertex *vertices, size_t offset, size_t size)
{
    for (auto child : children) {
        offset = child->BackingPipelineRender(vertices, offset, size);
    }
    return offset;
}


}}
