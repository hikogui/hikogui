// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include <filesystem>
#include <memory>

namespace TTauri::GUI::Widgets {

class ToolbarButtonWidget;
class WindowTrafficLightsWidget;

class WindowToolbarWidget : public Widget {
public:
    WindowTrafficLightsWidget *trafficLightButtons = nullptr;
    ToolbarButtonWidget *closeWindowButton = nullptr;
    ToolbarButtonWidget *maximizeWindowButton = nullptr;
    ToolbarButtonWidget *minimizeWindowButton = nullptr;

    WindowToolbarWidget();
    ~WindowToolbarWidget() {}

    WindowToolbarWidget(const WindowToolbarWidget &) = delete;
    WindowToolbarWidget &operator=(const WindowToolbarWidget &) = delete;
    WindowToolbarWidget(WindowToolbarWidget &&) = delete;
    WindowToolbarWidget &operator=(WindowToolbarWidget &&) = delete;

    virtual void setParent(Widget *parent);

    void pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, int &offset) override;

    HitBox hitBoxTest(glm::vec2 position) const override;

private:
    PipelineImage::Backing::ImagePixelMap drawImage(std::shared_ptr<GUI::PipelineImage::Image> image);

    PipelineImage::Backing backingImage; 
};

}