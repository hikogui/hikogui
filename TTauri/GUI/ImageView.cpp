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

size_t ImageView::backingPipelineRender(const gsl::span<BackingPipeline_vulkan::Vertex> &vertices, size_t offset)
{
    vertices.at(offset++) = { {position + glm::vec3(0.0,      0.0,      0.0)}, {0.0, 0.0, 0.0}, 1.0 };
    vertices.at(offset++) = { {position + glm::vec3(extent.x, 0.0,      0.0)}, {0.0, 0.0, 0.0}, 1.0 };
    vertices.at(offset++) = { {position + glm::vec3(extent.x, extent.y, 0.0)}, {0.0, 0.0, 0.0}, 1.0 };
    vertices.at(offset++) = { {position + glm::vec3(0.0,      0.0,      0.0)}, {0.0, 0.0, 0.0}, 1.0 };
    vertices.at(offset++) = { {position + glm::vec3(extent.x, extent.y, 0.0)}, {0.0, 0.0, 0.0}, 1.0 };
    vertices.at(offset++) = { {position + glm::vec3(0.0,      extent.y, 0.0)}, {0.0, 0.0, 0.0}, 1.0 };

    return offset;
}

}}
