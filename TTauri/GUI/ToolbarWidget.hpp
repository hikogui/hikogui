// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include <filesystem>
#include <memory>

namespace TTauri::GUI {

namespace PipelineImage {
struct Image;
struct Vertex;
}

class WindowDecorationWidget;
class WindowTrafficLightsWidget;

class ToolbarWidget : public Widget {
public:
    std::shared_ptr<PipelineImage::Image> backingImage;

    WindowTrafficLightsWidget *leftDecorationWidget = nullptr;
    WindowDecorationWidget *rightDecorationWidget = nullptr;

    ToolbarWidget();
    ~ToolbarWidget() {}

    ToolbarWidget(const ToolbarWidget &) = delete;
    ToolbarWidget &operator=(const ToolbarWidget &) = delete;
    ToolbarWidget(ToolbarWidget &&) = delete;
    ToolbarWidget &operator=(ToolbarWidget &&) = delete;

    virtual void setParent(Widget *parent);

    void drawBackingImage();

    void pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, size_t &offset) override;
};

}