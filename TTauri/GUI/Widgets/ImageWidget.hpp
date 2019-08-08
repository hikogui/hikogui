// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include <memory>

namespace TTauri::GUI::Widgets {

class ImageWidget : public Widget {
public:
    const URL path;

    float rotation = 0.0;

    std::shared_ptr<GUI::PipelineImage::Image> backingImage;

    ImageWidget(const URL path) noexcept;
    ~ImageWidget() {}

    ImageWidget(const ImageWidget&) = delete;
    ImageWidget&operator=(const ImageWidget&) = delete;
    ImageWidget(ImageWidget&&) = delete;
    ImageWidget&operator=(ImageWidget&&) = delete;

    void drawBackingImage() noexcept;

    void pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex> &vertices, int &offset) noexcept override;

private:
    std::string key;
};

}
