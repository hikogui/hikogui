//
//  ImageView.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "View.hpp"

#include <boost/filesystem.hpp>

#include <memory>

namespace TTauri {
namespace GUI {

class ImageView : public View {
public:
    const boost::filesystem::path path;

    ImageView(const boost::filesystem::path &path);
    ~ImageView() {}

    ImageView(const ImageView &) = delete;
    ImageView &operator=(const ImageView &) = delete;
    ImageView(ImageView &&) = delete;
    ImageView &operator=(ImageView &&) = delete;

    size_t piplineRectangledFromAtlasPlaceVertices(const gsl::span<PipelineRectanglesFromAtlas::Vertex> &vertices, size_t offset) override;
};

}}
