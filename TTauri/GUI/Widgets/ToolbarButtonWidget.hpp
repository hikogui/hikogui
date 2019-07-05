// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "TTauri/Draw/Path.hpp"
#include <memory>
#include <string>
#include <array>
#include <variant>

namespace TTauri::GUI::Widgets {

class ToolbarButtonWidget : public Widget {
public:
    bool enabled = true;
    bool hover = false;
    bool pressed = false;

    std::variant<Draw::Path> icon;

    ToolbarButtonWidget(Draw::Path const icon);
    ~ToolbarButtonWidget() {}

    ToolbarButtonWidget(const ToolbarButtonWidget &) = delete;
    ToolbarButtonWidget &operator=(const ToolbarButtonWidget &) = delete;
    ToolbarButtonWidget(ToolbarButtonWidget &&) = delete;
    ToolbarButtonWidget &operator=(ToolbarButtonWidget &&) = delete;

    void pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex>& vertices, size_t& offset) override;

    void handleMouseEvent(GUI::MouseEvent event) override;

protected:
    void drawImage(GUI::PipelineImage::Image &image);
private:
    int state() const;

    std::shared_ptr<GUI::PipelineImage::Image> image;

    // Shared key to reduce number of allocations.
    BinaryKey key;
};

}
