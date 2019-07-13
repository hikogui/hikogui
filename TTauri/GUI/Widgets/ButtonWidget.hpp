// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri {
struct wsRGBA;
}
namespace TTauri::Draw {
template<typename T>
struct PixelMap;
}

namespace TTauri::GUI::Widgets {

class ButtonWidget : public Widget {
public:
    bool value = false;
    bool enabled = true;
    bool focus = false;
    bool pressed = false;

    std::string label;

    ButtonWidget(std::string const label);
    ~ButtonWidget() {}

    ButtonWidget(const ButtonWidget &) = delete;
    ButtonWidget &operator=(const ButtonWidget &) = delete;
    ButtonWidget(ButtonWidget&&) = delete;
    ButtonWidget &operator=(ButtonWidget &&) = delete;

    int state() {
        int r = 0;
        r |= value ? 1 : 0;
        r |= enabled ? 2 : 0;
        r |= focus ? 4 : 0;
        r |= pressed ? 8 : 0;
        return r;
    }

    void pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex>& vertices, size_t& offset) override;

    void handleMouseEvent(GUI::MouseEvent event) override;

private:
    using ImagePixelMap = std::pair<std::shared_ptr<GUI::PipelineImage::Image>,Draw::PixelMap<wsRGBA>>;

    ImagePixelMap drawImage(std::shared_ptr<GUI::PipelineImage::Image> image);
        
    std::optional<std::future<ImagePixelMap>> pixelMapFuture;
    std::shared_ptr<GUI::PipelineImage::Image> image;

    // Shared key to reduce number of allocations.
    std::string key;
};

}
