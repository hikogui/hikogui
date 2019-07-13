// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "TTauri/Draw/Path.hpp"
#include <memory>
#include <string>
#include <array>

namespace TTauri::Draw {
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

    Draw::Path applicationIcon;

    std::shared_ptr<PipelineImage::Image> image;

    WindowTrafficLightsWidget(Draw::Path applicationIcon);
    ~WindowTrafficLightsWidget() {}

    WindowTrafficLightsWidget(const WindowTrafficLightsWidget &) = delete;
    WindowTrafficLightsWidget &operator=(const WindowTrafficLightsWidget &) = delete;
    WindowTrafficLightsWidget(WindowTrafficLightsWidget &&) = delete;
    WindowTrafficLightsWidget &operator=(WindowTrafficLightsWidget &&) = delete;

    virtual void setParent(Widget *parent);

    int state() const;

    void pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex>& vertices, size_t& offset) override;

    void handleMouseEvent(MouseEvent event) override;
    HitBox hitBoxTest(glm::vec2 position) const override;

protected:
    void drawImage(PipelineImage::Image &image);

private:
    std::tuple<rect2, rect2, rect2, rect2> getButtonRectangles() const;

    void drawApplicationIconImage(PipelineImage::Image &image);
    void drawTrafficLightsImage(PipelineImage::Image &image);

    static void drawCross(Draw::Path &path, glm::vec2 position, float radius);
    static void drawTrianglesOutward(Draw::Path &path, glm::vec2 position, float radius);
    static void drawTrianglesInward(Draw::Path &path, glm::vec2 position, float radius);

    // Shared key to reduce number of allocations.
    std::string key;
};

}
