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

class ButtonWidget : public Widget {
protected:
    bool value = false;
    bool pressed = false;

    std::string label = "<unknown>";

    Text::ShapedText labelShapedText;
public:

    ButtonWidget(std::string const label) noexcept;
    ~ButtonWidget() {}

    ButtonWidget(const ButtonWidget &) = delete;
    ButtonWidget &operator=(const ButtonWidget &) = delete;
    ButtonWidget(ButtonWidget&&) = delete;
    ButtonWidget &operator=(ButtonWidget &&) = delete;

    [[nodiscard]] bool updateAndPlaceVertices(
        vspan<PipelineFlat::Vertex> &flat_vertices,
        vspan<PipelineBox::Vertex> &box_vertices,
        vspan<PipelineImage::Vertex> &image_vertices,
        vspan<PipelineSDF::Vertex> &sdf_vertices
    ) noexcept override;

    [[nodiscard]] bool handleMouseEvent(GUI::MouseEvent const &event) noexcept override;
    [[nodiscard]] bool handleKeyboardEvent(GUI::KeyboardEvent const &event) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) noexcept;
    [[nodiscard]] bool acceptsFocus() noexcept override {
        return enabled;
    }

};

}
