//
//  ImageView.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "ImageView.hpp"

namespace TTauri {
namespace GUI {

ImageView::ImageView(const boost::filesystem::path &path) :
    View(), path(path)
{
}

ImageView::~ImageView()
{
}

size_t ImageView::BackingPipelineRender(BackingPipeline::Vertex *vertices, size_t offset, size_t size)
{
    if (offset + 6 >= size) {
        BOOST_THROW_EXCEPTION(BackingPipeline::Delegate::Error());
    }

    vertices[offset++] = { {position + glm::vec3(0.0,      0.0,      0.0)}, {0.0, 0.0, 0.0}, 1.0 };
    vertices[offset++] = { {position + glm::vec3(extent.x, 0.0,      0.0)}, {0.0, 0.0, 0.0}, 1.0 };
    vertices[offset++] = { {position + glm::vec3(extent.x, extent.y, 0.0)}, {0.0, 0.0, 0.0}, 1.0 };
    vertices[offset++] = { {position + glm::vec3(0.0,      0.0,      0.0)}, {0.0, 0.0, 0.0}, 1.0 };
    vertices[offset++] = { {position + glm::vec3(extent.x, extent.y, 0.0)}, {0.0, 0.0, 0.0}, 1.0 };
    vertices[offset++] = { {position + glm::vec3(0.0,      extent.y, 0.0)}, {0.0, 0.0, 0.0}, 1.0 };

    return offset;
}

}}
