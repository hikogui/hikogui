// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"

namespace TTauri::GUI {

class Window;

class WindowWidget : public Widget {
public:
    enum class Type {
        WINDOW,
        PANEL,
        FULLSCREEN,
    };

    WindowWidget(const std::weak_ptr<Window> window);
    ~WindowWidget(){};

    WindowWidget(const WindowWidget&) = delete;
    WindowWidget &operator=(const WindowWidget&) = delete;
    WindowWidget(WindowWidget&&) = delete;
    WindowWidget &operator=(WindowWidget&&) = delete;
};

}
