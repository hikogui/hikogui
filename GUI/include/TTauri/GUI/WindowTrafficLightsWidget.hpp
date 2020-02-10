// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include "TTauri/Foundation/Path.hpp"
#include <memory>
#include <string>
#include <array>

namespace TTauri {
struct Path;
}

namespace TTauri::GUI::Widgets {

class WindowTrafficLightsWidget : public Widget {
public:
    static constexpr float RADIUS = 5.5;
    static constexpr float DIAMETER = RADIUS * 2.0;
    static constexpr float MARGIN = 10.0;
    static constexpr float SPACING = 8.0;

    static constexpr float WIDTH = DIAMETER * 3.0 + 2.0 * MARGIN + 2 * SPACING;
    static constexpr float HEIGHT = DIAMETER + 2.0 * MARGIN;

    bool windowFocus = true;
    bool hover = false;
    bool pressedRed = false;
    bool pressedYellow = false;
    bool pressedGreen = false;

    Path applicationIcon;

    WindowTrafficLightsWidget(Path applicationIcon) noexcept;
    ~WindowTrafficLightsWidget() {}

    WindowTrafficLightsWidget(const WindowTrafficLightsWidget &) = delete;
    WindowTrafficLightsWidget &operator=(const WindowTrafficLightsWidget &) = delete;
    WindowTrafficLightsWidget(WindowTrafficLightsWidget &&) = delete;
    WindowTrafficLightsWidget &operator=(WindowTrafficLightsWidget &&) = delete;

    void setParent(Widget *parent) noexcept override;

    int state() const noexcept;

    void pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex>& vertices, ssize_t& offset) noexcept override;

    void handleMouseEvent(MouseEvent event) noexcept override;
    HitBox hitBoxTest(glm::vec2 position) const noexcept override;

private:
    std::tuple<rect2, rect2, rect2, rect2> getButtonRectangles() const noexcept;

    PixelMap<wsRGBA> drawApplicationIconImage(PipelineImage::Image &image) noexcept;
    PixelMap<wsRGBA> drawTrafficLightsImage(PipelineImage::Image &image) noexcept;

    static void drawCross(Path &path, glm::vec2 position, float radius) noexcept;
    static void drawTrianglesOutward(Path &path, glm::vec2 position, float radius) noexcept;
    static void drawTrianglesInward(Path &path, glm::vec2 position, float radius) noexcept;

    PipelineImage::Backing::ImagePixelMap drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept;

    PipelineImage::Backing backingImage; 
};

}
