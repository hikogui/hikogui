// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
#include <memory>

namespace TTauri::GUI::Widgets {

class ToolbarButtonWidget;
class WindowTrafficLightsWidget;

class ToolbarWidget : public Widget {
public:
    WindowTrafficLightsWidget *trafficLightButtons = nullptr;
    ToolbarButtonWidget *closeWindowButton = nullptr;
    ToolbarButtonWidget *maximizeWindowButton = nullptr;
    ToolbarButtonWidget *minimizeWindowButton = nullptr;

    ToolbarWidget(Window &window, Widget *parent) noexcept;
    ~ToolbarWidget() {}

    ToolbarWidget(const ToolbarWidget &) = delete;
    ToolbarWidget &operator=(const ToolbarWidget &) = delete;
    ToolbarWidget(ToolbarWidget &&) = delete;
    ToolbarWidget &operator=(ToolbarWidget &&) = delete;

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;
};

}
