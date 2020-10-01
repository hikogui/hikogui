// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include "../cells/Label.hpp"

namespace tt {

class ToolbarWidget;
class GridWidget;

class WindowWidget final : public ContainerWidget {
public:
    WindowWidget(Window &window, GridWidgetDelegate *delegate, Label title) noexcept;
    ~WindowWidget();

    [[nodiscard]] bool updateConstraints() noexcept override;
    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept;
    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override;

    [[nodiscard]] GridWidget *content() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(_content);
        return _content;
    }

    [[nodiscard]] ToolbarWidget *toolbar() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(_toolbar);
        return _toolbar;
    }

private:
    Label title;
    GridWidget *_content = nullptr;
    ToolbarWidget *_toolbar = nullptr;
};

} // namespace tt
