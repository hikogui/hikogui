// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include "TTauri/Foundation/Path.hpp"
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

    std::variant<Path> icon;

    wsRGBA hoverBackgroundColor = wsRGBA{ 0xffffff11 };
    wsRGBA pressedBackgroundColor = wsRGBA{ 0xffffff22 };

    std::function<void()> delegate;

    ToolbarButtonWidget(Path const icon, std::function<void()> delegate) noexcept;
    ~ToolbarButtonWidget() {}

    ToolbarButtonWidget(const ToolbarButtonWidget &) = delete;
    ToolbarButtonWidget &operator=(const ToolbarButtonWidget &) = delete;
    ToolbarButtonWidget(ToolbarButtonWidget &&) = delete;
    ToolbarButtonWidget &operator=(ToolbarButtonWidget &&) = delete;

    void setParent(Widget *parent) noexcept override;

    void update(
        bool modified,
        vspan<PipelineFlat::Vertex> &flat_vertices,
        vspan<PipelineImage::Vertex> &image_vertices,
        vspan<PipelineSDF::Vertex> &sdf_vertices
    ) noexcept override;

    void handleMouseEvent(GUI::MouseEvent event) noexcept override;

private:
    int state() const noexcept;

    PipelineImage::Backing::ImagePixelMap drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept;

    PipelineImage::Backing backingImage; 
};

}
