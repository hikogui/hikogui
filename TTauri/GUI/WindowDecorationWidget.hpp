// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "Mouse.hpp"
#include "TTauri/Draw/attributes.hpp"
#include "TTauri/BinaryKey.hpp"
#include <memory>
#include <string>
#include <array>

namespace TTauri::GUI {

namespace PipelineImage {
struct Image;
struct Vertex;
}

class WindowDecorationWidget : public Widget {
public:
    Draw::Alignment alignment;
    bool value = false;
    bool enabled = true;
    bool focus = false;
    bool pressed = false;

    std::shared_ptr<PipelineImage::Image> image;

    WindowDecorationWidget(Draw::Alignment);
    ~WindowDecorationWidget() {}

    WindowDecorationWidget(const WindowDecorationWidget &) = delete;
    WindowDecorationWidget &operator=(const WindowDecorationWidget &) = delete;
    WindowDecorationWidget(WindowDecorationWidget &&) = delete;
    WindowDecorationWidget &operator=(WindowDecorationWidget &&) = delete;

    virtual void setParent(Widget *parent);

    int state() {
        int r = 0;
        r |= value ? 1 : 0;
        r |= enabled ? 2 : 0;
        r |= focus ? 4 : 0;
        r |= pressed ? 8 : 0;
        return r;
    }

    void pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex>& vertices, size_t& offset) override;

    void handleMouseEvent(MouseEvent event) override;

protected:
    void drawImage(PipelineImage::Image &image);
private:

    // Shared key to reduce number of allocations.
    BinaryKey key;
};

}
