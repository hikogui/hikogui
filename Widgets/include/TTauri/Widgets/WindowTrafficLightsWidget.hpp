// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
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

    bool pressedRed = false;
    bool pressedYellow = false;
    bool pressedGreen = false;

    Path applicationIcon;

    WindowTrafficLightsWidget(Window &window, Widget *parent, Path applicationIcon) noexcept;
    ~WindowTrafficLightsWidget() {}

    WindowTrafficLightsWidget(const WindowTrafficLightsWidget &) = delete;
    WindowTrafficLightsWidget &operator=(const WindowTrafficLightsWidget &) = delete;
    WindowTrafficLightsWidget(WindowTrafficLightsWidget &&) = delete;
    WindowTrafficLightsWidget &operator=(WindowTrafficLightsWidget &&) = delete;

    int state() const noexcept;

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    void handleMouseEvent(MouseEvent const &event) noexcept override;
    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;

private:
    std::tuple<aarect, aarect, aarect, aarect> getButtonRectangles() const noexcept;

    PixelMap<R16G16B16A16SFloat> drawApplicationIconImage(PipelineImage::Image &image) noexcept;
    PixelMap<R16G16B16A16SFloat> drawTrafficLightsImage(PipelineImage::Image &image) noexcept;

    static void drawCross(Path &path, vec position, float radius) noexcept;
    static void drawTrianglesOutward(Path &path, vec position, float radius) noexcept;
    static void drawTrianglesInward(Path &path, vec position, float radius) noexcept;

    PipelineImage::Backing::ImagePixelMap drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept;

    PipelineImage::Backing backingImage; 
};

}
