// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include "../cells/Label.hpp"

namespace tt {

class ToolbarWidget;

class WindowWidget : public ContainerWidget {
    Label title;

public:
    ContainerWidget *content = nullptr;
    ToolbarWidget *toolbar = nullptr;

    WindowWidget(Window &window, ContainerWidgetDelegate *delegate, Label title) noexcept;
    ~WindowWidget();

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;
};

}
