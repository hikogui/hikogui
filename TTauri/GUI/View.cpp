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

namespace TTauri::GUI {

View::View()
{
}

void View::setParent(const std::shared_ptr<View> &parent)
{
    this->window = parent->window;
    this->parent = move(parent);
}

void View::setRectangle(glm::vec2 position, u64extent2 extent)
{
    this->position = position;
    this->extent = extent;
}

void View::add(std::shared_ptr<View> view)
{
    view->setParent(shared_from_this());
    children.push_back(move(view));
}

void View::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, size_t &offset)
{
    for (auto child : children) {
        child->pipelineImagePlaceVertices(vertices, offset);
    }
}


}