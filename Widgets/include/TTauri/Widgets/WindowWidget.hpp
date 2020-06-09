// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"

namespace TTauri {

class ToolbarWidget;

class WindowWidget : public Widget {
public:
    ToolbarWidget *toolbar = nullptr;

    WindowWidget(Window &window) noexcept;

    WindowWidget(const WindowWidget&) = delete;
    WindowWidget &operator=(const WindowWidget&) = delete;
    WindowWidget(WindowWidget&&) = delete;
    WindowWidget &operator=(WindowWidget&&) = delete;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;
};

}
