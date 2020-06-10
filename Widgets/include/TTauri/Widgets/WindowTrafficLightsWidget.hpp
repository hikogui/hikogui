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

namespace TTauri {

class WindowTrafficLightsWidget : public Widget {
public:
    static constexpr float GLYPH_SIZE = 5.0f;
    static constexpr float RADIUS = 5.5f;
    static constexpr float DIAMETER = RADIUS * 2.0f;
    static constexpr float MARGIN = 10.0f;
    static constexpr float SPACING = 8.0f;

    aarect closeRectangle;
    aarect minimizeRectangle;
    aarect maximizeRectangle;

    FontGlyphIDs closeWindowGlyph;
    FontGlyphIDs minimizeWindowGlyph;
    FontGlyphIDs maximizeWindowGlyph;
    FontGlyphIDs restoreWindowGlyph;

    aarect closeWindowGlyphRectangle;
    aarect minimizeWindowGlyphRectangle;
    aarect maximizeWindowGlyphRectangle;
    aarect restoreWindowGlyphRectangle;

    bool hoverClose = false;
    bool hoverMinimize = false;
    bool hoverMaximize = false;

    bool pressedClose = false;
    bool pressedMinimize = false;
    bool pressedMaximize = false;

    WindowTrafficLightsWidget(Window &window, Widget *parent) noexcept;
    ~WindowTrafficLightsWidget() {}

    WindowTrafficLightsWidget(const WindowTrafficLightsWidget &) = delete;
    WindowTrafficLightsWidget &operator=(const WindowTrafficLightsWidget &) = delete;
    WindowTrafficLightsWidget(WindowTrafficLightsWidget &&) = delete;
    WindowTrafficLightsWidget &operator=(WindowTrafficLightsWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override;
    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    void handleMouseEvent(MouseEvent const &event) noexcept override;
    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;

private:
    static vec WindowTrafficLightsWidget::calculateExtent(Window &window) noexcept;

    void drawMacOS(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept;
    void drawWindows(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept;

};

}
