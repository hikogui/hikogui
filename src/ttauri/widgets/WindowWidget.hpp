// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include "../cells/Label.hpp"

namespace tt {

class ToolbarWidget;
class GridWidget;

class WindowWidget : public ContainerWidget {
    Label title;

public:
    GridWidget *content = nullptr;
    ToolbarWidget *toolbar = nullptr;

    WindowWidget(Window &window, GridWidgetDelegate *delegate, Label title) noexcept;
    ~WindowWidget();

    [[nodiscard]] bool updateConstraints() noexcept override;
    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;
};

} // namespace tt
