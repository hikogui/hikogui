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
    toolbar = &makeWidget<ToolbarWidget, ""_ca>();

    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
        toolbar->makeWidget<SystemMenuWidget>(this->title.icon());
#endif
        toolbar->makeWidget<WindowTrafficLightsWidget, "R0T0"_ca>();
    } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        toolbar->makeWidget<WindowTrafficLightsWidget>();
    } else {
        tt_no_default;
    }

    content = &makeWidget<ColumnWidget, ""_ca>();
    content->elevation = elevation;
}

WindowWidget::~WindowWidget() {}

[[nodiscard]] WidgetUpdateResult WindowWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (ttlet result = ContainerWidget::updateConstraints(); result < WidgetUpdateResult::Children) {
        return result;
    }

    ttlet toolbar_lock = std::scoped_lock(toolbar->mutex);
    ttlet toolbar_size = toolbar->size();

    ttlet content_lock = std::scoped_lock(content->mutex);
    _size = merge(content->size() + toolbar_size._0y(), toolbar_size.x0());
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

} // namespace tt
