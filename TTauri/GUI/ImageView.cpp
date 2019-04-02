//
//  ImageView.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "ImageView.hpp"

namespace TTauri::GUI {

ImageView::ImageView(const boost::filesystem::path path) :
    View(), path(std::move(path))
{
}

size_t ImageView::piplineRectangledFromAtlasPlaceVertices(const gsl::span<PipelineImage::Vertex> &vertices, size_t offset)
{
    PipelineImage::Vertex v;

    v.position = position + glm::vec2(0.0,      0.0     );
    vertices.at(offset++) = v;
    v.position = position + glm::vec2(extent.x, 0.0     );
    vertices.at(offset++) = v;
    v.position = position + glm::vec2(0.0,      extent.y);
    vertices.at(offset++) = v;
    v.position = position + glm::vec2(extent.x, extent.y);
    vertices.at(offset++) = v;

    return offset;
}

}
