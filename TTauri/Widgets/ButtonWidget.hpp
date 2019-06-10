// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/all.hpp"
#include <memory>
#include <string>
#include <array>

namespace TTauri::Widgets {

class ButtonWidget : public GUI::Widget {
public:
    bool value = false;
    bool enabled = true;
    bool focus = false;
    bool pressed = false;

    std::shared_ptr<GUI::PipelineImage::Image> image;

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

protected:
    void drawImage(GUI::PipelineImage::Image &image);
private:

    // Shared key to reduce number of allocations.
    BinaryKey key;
};

}
