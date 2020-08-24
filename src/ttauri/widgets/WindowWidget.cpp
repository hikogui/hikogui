// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowWidget.hpp"
#include "WindowTrafficLightsWidget.hpp"
#include "ToolbarWidget.hpp"
#include "ColumnWidget.hpp"
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "SystemMenuWidget.hpp"
#endif
#include "../GUI/utils.hpp"

namespace tt {

using namespace std;

WindowWidget::WindowWidget(Window &window, ContainerWidgetDelegate *delegate, Label title) noexcept :
    ContainerWidget(window, nullptr, delegate), title(std::move(title))
{
    toolbar = &makeWidget<ToolbarWidget,""_ca>();
    window.addConstraint(toolbar->left == left);
    window.addConstraint(toolbar->right == right);
    window.addConstraint(toolbar->top == top);

    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
        toolbar->makeWidget<SystemMenuWidget>(this->title.icon());
#endif
        toolbar->makeWidget<WindowTrafficLightsWidget,"R0T0"_ca>();
    } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        toolbar->makeWidget<WindowTrafficLightsWidget>();
    } else {
        tt_no_default;
    }


    content = &makeWidget<ColumnWidget,""_ca>();
    content->elevation = elevation;
    window.addConstraint(content->left == left + Theme::margin);
    window.addConstraint(content->right == right - Theme::margin);
    window.addConstraint(content->top == toolbar->bottom - Theme::margin);
    window.addConstraint(content->bottom == bottom + Theme::margin);

    // Add constraints for the window widget itself.
    leftConstraint = window.addConstraint(left == 0);
    bottomConstraint = window.addConstraint(bottom == 0);
    // A upper bound constraint is needed to allow the suggest(width, limit::max()) and suggest(height, limit::max()) to
    // fallback on a upper bound, otherwise it will select the lower bounds instead.
    maximumWidthConstraint = window.addConstraint(width <= std::numeric_limits<uint16_t>::max());
    minimumHeightConstraint = window.addConstraint(height <= std::numeric_limits<uint16_t>::max());
}

WindowWidget::~WindowWidget()
{
    window.removeConstraint(leftConstraint);
    window.removeConstraint(bottomConstraint);
    window.removeConstraint(maximumWidthConstraint);
    window.removeConstraint(maximumHeightConstraint);
    window.removeConstraint(widthConstraint);
    window.removeConstraint(heightConstraint);
}

[[nodiscard]] WidgetUpdateResult WindowWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (ttlet result = ContainerWidget::updateConstraints(); result < WidgetUpdateResult::Children) {
        return result;
    }
    
    window.stopConstraintSolver();
    window.replaceConstraint(widthConstraint, width == 0.0, rhea::strength::strong());
    window.replaceConstraint(heightConstraint, height == 0.0, rhea::strength::strong());
    window.startConstraintSolver();
    _minimumExtent = vec{width.value(), height.value()};

    window.stopConstraintSolver();
    window.replaceConstraint(widthConstraint, width == 0.0, rhea::strength::weak());
    window.replaceConstraint(heightConstraint, height == 0.0, rhea::strength::weak());
    window.startConstraintSolver();
    _preferredExtent = vec{width.value(), height.value()};

    window.stopConstraintSolver();
    window.replaceConstraint(widthConstraint, width == std::numeric_limits<float>::max(), rhea::strength::strong());
    window.replaceConstraint(heightConstraint, height == std::numeric_limits<float>::max(), rhea::strength::strong());
    window.startConstraintSolver();
    _maximumExtent = vec{width.value(), height.value()};

    window.stopConstraintSolver();
    window.replaceConstraint(widthConstraint, width == _windowExtent.width(), rhea::strength::strong());
    window.replaceConstraint(heightConstraint, height == _windowExtent.height(), rhea::strength::strong());
    window.startConstraintSolver();
    _windowExtent = vec{width.value(), height.value()};

    return WidgetUpdateResult::Self;
}

HitBox WindowWidget::hitBoxTest(vec position) const noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    constexpr float BORDER_WIDTH = 5.0;

    auto r = HitBox{this, elevation};

    if (position.x() <= BORDER_WIDTH) {
        if (position.y() <= BORDER_WIDTH) {
            r.type = HitBox::Type::BottomLeftResizeCorner;
        } else if (position.y() >= (rectangle().height() - BORDER_WIDTH)) {
            r.type = HitBox::Type::TopLeftResizeCorner;
        } else {
            r.type = HitBox::Type::LeftResizeBorder;
        }

    } else if (position.x() >= (rectangle().width() - BORDER_WIDTH)) {
        if (position.y() <= BORDER_WIDTH) {
            r.type = HitBox::Type::BottomRightResizeCorner;
        } else if (position.y() >= (rectangle().height() - BORDER_WIDTH)) {
            r.type = HitBox::Type::TopRightResizeCorner;
        } else {
            r.type = HitBox::Type::RightResizeBorder;
        }

    } else if (position.y() <= BORDER_WIDTH) {
        r.type = HitBox::Type::BottomResizeBorder;

    } else if (position.y() >= (rectangle().height() - BORDER_WIDTH)) {
        r.type = HitBox::Type::TopResizeBorder;
    }

    if (r.type != HitBox::Type::Outside) {
        // Resize corners need to override anything else, so that it is
        // always possible to resize a window.
        return r;
    }

    for (ttlet &child : children) {
        ttlet child_lock = std::scoped_lock(child->mutex);
        r = std::max(r, child->hitBoxTest(position - child->offsetFromParent));
    }

    return r;
}

}
