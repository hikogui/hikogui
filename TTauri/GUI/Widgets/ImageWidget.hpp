// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include <filesystem>
#include <memory>

namespace TTauri::GUI::Widgets {

class ImageWidget : public Widget {
public:
    const std::filesystem::path path;

    float rotation = 0.0;

    std::shared_ptr<GUI::PipelineImage::Image> backingImage;

    ImageWidget(const std::filesystem::path path);
    ~ImageWidget() {}

    ImageWidget(const ImageWidget&) = delete;
    ImageWidget&operator=(const ImageWidget&) = delete;
    ImageWidget(ImageWidget&&) = delete;
    ImageWidget&operator=(ImageWidget&&) = delete;

    void drawBackingImage();

    void pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex> &vertices, size_t &offset) override;
};

}
