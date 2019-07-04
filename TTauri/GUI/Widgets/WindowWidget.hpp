// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"

namespace TTauri::GUI::Widgets {

class WindowToolbarWidget;
class WindowTrafficLightsWidget;

class WindowWidget : public Widget {
public:
    enum class Type {
        WINDOW,
        PANEL,
        FULLSCREEN,
    };

    WindowTrafficLightsWidget *leftDecorationWidget = nullptr;
    WindowToolbarWidget *toolbar = nullptr;

    wsRGBA backgroundColor;

    WindowWidget();
    ~WindowWidget(){};

    WindowWidget(const WindowWidget&) = delete;
    WindowWidget &operator=(const WindowWidget&) = delete;
    WindowWidget(WindowWidget&&) = delete;
    WindowWidget &operator=(WindowWidget&&) = delete;

    void setParent(Window *window);

    HitBox hitBoxTest(glm::vec2 position) const override;
};

}
