// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri::GUI::Widgets {

class ButtonWidget : public Widget {
public:
    bool value = false;
    bool enabled = true;
    bool focus = false;
    bool pressed = false;

    std::string label;

    ButtonWidget(std::string const label) noexcept;
    ~ButtonWidget() {}

    ButtonWidget(const ButtonWidget &) = delete;
    ButtonWidget &operator=(const ButtonWidget &) = delete;
    ButtonWidget(ButtonWidget&&) = delete;
    ButtonWidget &operator=(ButtonWidget &&) = delete;

    int state() const noexcept{
        int r = 0;
        r |= value ? 1 : 0;
        r |= enabled ? 2 : 0;
        r |= focus ? 4 : 0;
        r |= pressed ? 8 : 0;
        return r;
    }

    void pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex>& vertices, int& offset) noexcept override;

    void handleMouseEvent(GUI::MouseEvent event) noexcept override;


private:
    PipelineImage::Backing::ImagePixelMap drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept;

    PipelineImage::Backing backingImage; 
};

}
