// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include <memory>

namespace TTauri::GUI::Widgets {

class ImageWidget : public Widget {
public:
    const URL path;

    float rotation = 0.0;

    std::shared_ptr<GUI::PipelineImage::Image> backingImage;

    ImageWidget(Window &window, Widget *parent, const URL path) noexcept;
    ~ImageWidget() {}

    ImageWidget(const ImageWidget&) = delete;
    ImageWidget&operator=(const ImageWidget&) = delete;
    ImageWidget(ImageWidget&&) = delete;
    ImageWidget&operator=(ImageWidget&&) = delete;

    void drawBackingImage() noexcept;

    [[nodiscard]] bool updateAndPlaceVertices(
        vspan<PipelineFlat::Vertex> &flat_vertices,
        vspan<PipelineBox::Vertex> &box_vertices,
        vspan<PipelineImage::Vertex> &image_vertices,
        vspan<PipelineSDF::Vertex> &sdf_vertices
    ) noexcept override;

private:
    std::string key;
};

}
