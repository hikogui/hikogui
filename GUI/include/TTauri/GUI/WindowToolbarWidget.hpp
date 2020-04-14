// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include <memory>

namespace TTauri::GUI::Widgets {

class ToolbarButtonWidget;
class WindowTrafficLightsWidget;

class WindowToolbarWidget : public Widget {
public:
    WindowTrafficLightsWidget *trafficLightButtons = nullptr;
    ToolbarButtonWidget *closeWindowButton = nullptr;
    ToolbarButtonWidget *maximizeWindowButton = nullptr;
    ToolbarButtonWidget *minimizeWindowButton = nullptr;

    vec backgroundColor = {0.0f, 0.0f, 0.0f, 0.5f};

    WindowToolbarWidget(Window &window, Widget *parent) noexcept;
    ~WindowToolbarWidget() {}

    WindowToolbarWidget(const WindowToolbarWidget &) = delete;
    WindowToolbarWidget &operator=(const WindowToolbarWidget &) = delete;
    WindowToolbarWidget(WindowToolbarWidget &&) = delete;
    WindowToolbarWidget &operator=(WindowToolbarWidget &&) = delete;

    void draw(DrawContext &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) noexcept override;
};

}
