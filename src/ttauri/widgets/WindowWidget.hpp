// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/widgets/Widget.hpp"
#include "ttauri/cells/Label.hpp"

namespace tt {

class ToolbarWidget;

class WindowWidget : public Widget {
    Label title;

public:
    ToolbarWidget *toolbar = nullptr;

    WindowWidget(Window &window, Label title) noexcept;

    WindowWidget(const WindowWidget&) = delete;
    WindowWidget &operator=(const WindowWidget&) = delete;
    WindowWidget(WindowWidget&&) = delete;
    WindowWidget &operator=(WindowWidget&&) = delete;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;
};

}
