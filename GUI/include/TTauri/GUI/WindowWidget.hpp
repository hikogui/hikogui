// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"

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

    vec backgroundColor;

    WindowWidget() noexcept;

    WindowWidget(const WindowWidget&) = delete;
    WindowWidget &operator=(const WindowWidget&) = delete;
    WindowWidget(WindowWidget&&) = delete;
    WindowWidget &operator=(WindowWidget&&) = delete;

    void setParentWindow(gsl::not_null<Window *> window) noexcept;

    [[nodiscard]] HitBox hitBoxTest(vec position) noexcept override;
};

}
