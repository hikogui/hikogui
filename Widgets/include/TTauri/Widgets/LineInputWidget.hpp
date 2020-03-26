// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include "TTauri/Text/EditableText.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri::GUI::Widgets {

class LineInputWidget : public Widget {
protected:
    std::string label = "<unknown>";

    Text::EditableText field;
    Text::ShapedText shapedText;

    vec leftCaretPosition = vec{};
    vec rightCaretPosition = vec{};
public:

    LineInputWidget(
        std::string const label,
        Text::TextStyle style=Text::TextStyle(
            "Arial",
            Text::FontVariant{Text::FontWeight::Regular, false},
            14.0,
            vec::color(1.0,1.0,1.0),
            0.0,
            Text::TextDecoration::None
        )
    ) noexcept;

    ~LineInputWidget() {}

    LineInputWidget(const LineInputWidget &) = delete;
    LineInputWidget &operator=(const LineInputWidget &) = delete;
    LineInputWidget(LineInputWidget&&) = delete;
    LineInputWidget &operator=(LineInputWidget &&) = delete;

    [[nodiscard]] bool updateAndPlaceVertices(
        vspan<PipelineFlat::Vertex> &flat_vertices,
        vspan<PipelineBox::Vertex> &box_vertices,
        vspan<PipelineImage::Vertex> &image_vertices,
        vspan<PipelineSDF::Vertex> &sdf_vertices
    ) noexcept override;

    [[nodiscard]] bool handleCommand(string_ltag command) noexcept;

    [[nodiscard]] bool handleMouseEvent(GUI::MouseEvent const &event) noexcept override;
    [[nodiscard]] bool handleKeyboardEvent(GUI::KeyboardEvent const &event) noexcept override;
    [[nodiscard]] HitBox hitBoxTest(vec position) noexcept;
    [[nodiscard]] bool acceptsFocus() noexcept override {
        return enabled;
    }

};

}
