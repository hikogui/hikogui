// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
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

    R16G16B16A16SFloat backgroundColor = vec{0.0f, 0.0f, 0.0f, 0.5f};

    WindowToolbarWidget() noexcept;
    ~WindowToolbarWidget() {}

    WindowToolbarWidget(const WindowToolbarWidget &) = delete;
    WindowToolbarWidget &operator=(const WindowToolbarWidget &) = delete;
    WindowToolbarWidget(WindowToolbarWidget &&) = delete;
    WindowToolbarWidget &operator=(WindowToolbarWidget &&) = delete;

    void setParent(Widget *parent) noexcept override;

    [[nodiscard]] bool updateAndPlaceVertices(
        bool modified,
        vspan<PipelineFlat::Vertex> &flat_vertices,
        vspan<PipelineBox::Vertex> &box_vertices,
        vspan<PipelineImage::Vertex> &image_vertices,
        vspan<PipelineSDF::Vertex> &sdf_vertices
    ) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(glm::vec2 position) const noexcept override;
};

}
