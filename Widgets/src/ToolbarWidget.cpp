// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ToolbarWidget.hpp"
#include "TTauri/Widgets/WindowTrafficLightsWidget.hpp"
#include "TTauri/Widgets/ToolbarButtonWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include <cmath>

namespace TTauri {

using namespace std;

ToolbarWidget::ToolbarWidget(Window &window, Widget *parent) noexcept :
    Widget(window, parent, vec{Theme::width, Theme::smallHeight})
{
    // Keep the toolbar thin.
    window.addConstraint(height <= Theme::smallHeight, rhea::strength::strong());
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
    if (ssize(leftChildren) != 0 && ssize(rightChildren) != 0) {
        leftRightJoinConstraint = window.addConstraint(leftChildren.back()->right <= rightChildren.back()->left);
    } else if (ssize(leftChildren) != 0) {
        leftRightJoinConstraint = window.addConstraint(leftChildren.back()->right <= right);
    } else if (ssize(rightChildren) != 0) {
        leftRightJoinConstraint = window.addConstraint(left <= rightChildren.back()->left);
    }
}

Widget &ToolbarWidget::addWidget(Alignment alignment, std::unique_ptr<Widget> childWidget) noexcept
{
    auto &tmp = Widget::addWidget(alignment, std::move(childWidget));

    disjoinLeftAndRightChildren();

    if (alignment == HorizontalAlignment::Right) {
        auto previousWidget = ssize(rightChildren) != 0 ? rightChildren.back() : nullptr;
        rightChildren.push_back(&tmp);

        if (previousWidget == nullptr) {
            tmp.placeRight(0.0);
        } else {
            tmp.placeLeftOf(*previousWidget);
        }

    } else {
        auto previousWidget = ssize(leftChildren) != 0 ? leftChildren.back() : nullptr;
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

    for (let &child : children) {
        r = std::max(r, child->hitBoxTest(position - child->offsetFromParent()));
    }
    return r;
}

}
