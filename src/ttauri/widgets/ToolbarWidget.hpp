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

    Widget &addWidget(WidgetPosition position, std::unique_ptr<Widget> childWidget) noexcept override;

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
