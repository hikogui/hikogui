// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "GridWidget.hpp"
#include <memory>

namespace tt {

class ToolbarWidget : public GridWidget {
public:
    ToolbarWidget(Window &window, Widget *parent) noexcept;
    ~ToolbarWidget() {}

    /** Add a widget directly to this widget.
    *
    * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
    */
    template<typename T, cell_address CellAddress="T0L+1"_ca, typename... Args>
    T &makeWidget(Args &&... args) {
        return ContainerWidget::makeWidget<T,CellAddress>(std::forward<Args>(args)...);
    }

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override;

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;
};

}
