// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include <memory>

namespace tt {

class ToolbarWidget : public Widget {
public:
    ToolbarWidget(Window &window, Widget *parent) noexcept;
    ~ToolbarWidget() {}

    ToolbarWidget(const ToolbarWidget &) = delete;
    ToolbarWidget &operator=(const ToolbarWidget &) = delete;
    ToolbarWidget(ToolbarWidget &&) = delete;
    ToolbarWidget &operator=(ToolbarWidget &&) = delete;

    Widget &addWidget(Alignment alignment, std::unique_ptr<Widget> childWidget) noexcept override;

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
