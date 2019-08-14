// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"

namespace TTauri::GUI::Widgets {

class WindowToolbarWidget;

class WindowWidget : public Widget {
public:
    enum class Type {
        WINDOW,
        PANEL,
        FULLSCREEN,
    };

    WindowToolbarWidget *toolbar = nullptr;

    wsRGBA backgroundColor;

    WindowWidget() noexcept;

    WindowWidget(const WindowWidget&) = delete;
    WindowWidget &operator=(const WindowWidget&) = delete;
    WindowWidget(WindowWidget&&) = delete;
    WindowWidget &operator=(WindowWidget&&) = delete;

    void setParent(gsl::not_null<Window *> window) noexcept;

    HitBox hitBoxTest(glm::vec2 position) const noexcept override;
};

}
