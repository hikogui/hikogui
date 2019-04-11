//
//  ImageView.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "View.hpp"

#include <filesystem>

#include <memory>

namespace TTauri::GUI {

class ImageView : public View {
public:
    const std::filesystem::path path;

    float rotation = 0.0;

    std::shared_ptr<PipelineImage::Image> backingImage;

    ImageView(const std::filesystem::path path);
    ~ImageView() {}

    ImageView(const ImageView &) = delete;
    ImageView &operator=(const ImageView &) = delete;
    ImageView(ImageView &&) = delete;
    ImageView &operator=(ImageView &&) = delete;

    void drawBackingImage();

    void pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, size_t &offset) override;
};

}
