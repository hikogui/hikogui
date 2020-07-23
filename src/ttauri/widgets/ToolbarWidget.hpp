// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include <memory>

namespace tt {

class ToolbarWidget : public ContainerWidget {
public:
    ToolbarWidget(Window &window, Widget *parent) noexcept;
    ~ToolbarWidget() {}

    Widget &addWidget(cell_address address, std::unique_ptr<Widget> childWidget) noexcept override;

    /** Add a widget directly to this widget.
    *
    * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
    */
    template<typename T, cell_address CellAddress="L+1"_ca, typename... Args>
    T &makeWidget(Args &&... args) {
        return ContainerWidget::makeWidget<T,CellAddress>(std::forward<Args>(args)...);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;

private:
    std::vector<Widget *> leftChildren;
    std::vector<Widget *> rightChildren;
    rhea::constraint leftRightJoinConstraint;

    void joinLeftAndRightChildren() noexcept;
    void disjoinLeftAndRightChildren() noexcept;
};

}
