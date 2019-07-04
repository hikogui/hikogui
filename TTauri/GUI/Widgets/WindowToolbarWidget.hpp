// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include <filesystem>
#include <memory>

namespace TTauri::GUI::Widgets {


class WindowToolbarWidget : public Widget {
public:
    std::shared_ptr<PipelineImage::Image> backingImage;

    WindowToolbarWidget();
    ~WindowToolbarWidget() {}

    WindowToolbarWidget(const WindowToolbarWidget &) = delete;
    WindowToolbarWidget &operator=(const WindowToolbarWidget &) = delete;
    WindowToolbarWidget(WindowToolbarWidget &&) = delete;
    WindowToolbarWidget &operator=(WindowToolbarWidget &&) = delete;

    virtual void setParent(Widget *parent);

    void drawBackingImage();

    void pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, size_t &offset) override;

    HitBox hitBoxTest(glm::vec2 position) const override;

};

}