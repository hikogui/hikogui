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
    ContainerWidget(window, parent)
{
}

[[nodiscard]] WidgetUpdateResult ToolbarWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (ttlet result = ContainerWidget::updateConstraints(); result < WidgetUpdateResult::Self) {
        return result;
    }

    window.replaceConstraint(maximumHeightConstraint, height <= Theme::toolbarHeight, rhea::strength::weak());
    return WidgetUpdateResult::Self;
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

Widget &ToolbarWidget::addWidget(cell_address address, std::unique_ptr<Widget> childWidget) noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    auto &tmp = ContainerWidget::addWidget(address, std::move(childWidget));

    disjoinLeftAndRightChildren();

    if (is_opposite<false>(current_address)) {
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
    tt_assume(mutex.is_locked_by_current_thread());

    auto context = drawContext;
    context.drawFilledQuad(rectangle());
    ContainerWidget::draw(drawContext, displayTimePoint);
}

HitBox ToolbarWidget::hitBoxTest(vec position) const noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto r = rectangle().contains(position) ?
        HitBox{this, elevation, HitBox::Type::MoveArea} :
        HitBox{};

    for (ttlet &child : children) {
        ttlet child_lock = std::scoped_lock(child->mutex);
        r = std::max(r, child->hitBoxTest(position - child->offsetFromParent));
    }
    return r;
}

}
