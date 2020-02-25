// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri::GUI::Widgets {

class LineInputWidget : public Widget {
protected:
    bool value = false;
    bool enabled = true;
    bool focus = false;
    bool pressed = false;

    std::string label = "<unknown>";

    Text::ShapedText labelShapedText;
public:

    LineInputWidget(std::string const label) noexcept;
    ~LineInputWidget() {}

    LineInputWidget(const LineInputWidget &) = delete;
    LineInputWidget &operator=(const LineInputWidget &) = delete;
    LineInputWidget(LineInputWidget&&) = delete;
    LineInputWidget &operator=(LineInputWidget &&) = delete;

    [[nodiscard]] bool updateAndPlaceVertices(
        bool modified,
        vspan<PipelineFlat::Vertex> &flat_vertices,
        vspan<PipelineBox::Vertex> &box_vertices,
        vspan<PipelineImage::Vertex> &image_vertices,
        vspan<PipelineSDF::Vertex> &sdf_vertices
    ) noexcept override;

    [[nodiscard]] bool handleMouseEvent(GUI::MouseEvent const &event) noexcept override;
};

}
