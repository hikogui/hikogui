// Copyright 2019 Pokitec
// All rights reserved.

#include "ToolbarWidget.hpp"
#include "WindowTrafficLightsWidget.hpp"
#include "ToolbarButtonWidget.hpp"
#include "../GUI/utils.hpp"
#include <cmath>

namespace tt {

using namespace std;

ToolbarWidget::ToolbarWidget(Window &window, Widget *parent) noexcept :
    Widget(window, parent, vec{Theme::width, Theme::toolbarHeight})
{
    // Keep the toolbar thin.
    window.addConstraint(height <= Theme::toolbarHeight, rhea::strength::strong());
}

void ToolbarWidget::disjoinLeftAndRightChildren() noexcept
{
    if (!leftRightJoinConstraint.is_nil()) {
        window.removeConstraint(leftRightJoinConstraint);
        leftRightJoinConstraint = {};
    }
}

void ToolbarWidget::joinLeftAndRightChildren() noexcept
{
    if (nonstd::ssize(leftChildren) != 0 && nonstd::ssize(rightChildren) != 0) {
        leftRightJoinConstraint = window.addConstraint(leftChildren.back()->right <= rightChildren.back()->left);
    } else if (nonstd::ssize(leftChildren) != 0) {
        leftRightJoinConstraint = window.addConstraint(leftChildren.back()->right <= right);
    } else if (nonstd::ssize(rightChildren) != 0) {
        leftRightJoinConstraint = window.addConstraint(left <= rightChildren.back()->left);
    }
}

Widget &ToolbarWidget::addWidget(Alignment alignment, std::unique_ptr<Widget> childWidget) noexcept
{
    auto &tmp = Widget::addWidget(alignment, std::move(childWidget));

    disjoinLeftAndRightChildren();

    if (alignment == HorizontalAlignment::Right) {
        auto previousWidget = nonstd::ssize(rightChildren) != 0 ? rightChildren.back() : nullptr;
        rightChildren.push_back(&tmp);

        if (previousWidget == nullptr) {
            tmp.placeRight(0.0);
        } else {
            tmp.placeLeftOf(*previousWidget);
        }

    } else {
        auto previousWidget = nonstd::ssize(leftChildren) != 0 ? leftChildren.back() : nullptr;
        leftChildren.push_back(&tmp);

        if (previousWidget == nullptr) {
            tmp.placeLeft(0.0);
        } else {
            tmp.placeRightOf(*previousWidget);
        }
    }
    tmp.placeAtTop(0.0f);
    tmp.placeAtBottom(0.0f);

    joinLeftAndRightChildren();
    return tmp;
}

void ToolbarWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    auto context = drawContext;

    context.drawFilledQuad(rectangle());

    Widget::draw(drawContext, displayTimePoint);
}

HitBox ToolbarWidget::hitBoxTest(vec position) const noexcept
{
    auto r = rectangle().contains(position) ?
        HitBox{this, elevation, HitBox::Type::MoveArea} :
        HitBox{};

    for (ttlet &child : children) {
        r = std::max(r, child->hitBoxTest(position - child->offsetFromParent()));
    }
    return r;
}

}
