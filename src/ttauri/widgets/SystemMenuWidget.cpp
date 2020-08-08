// Copyright 2020 Pokitec
// All rights reserved.

#include "SystemMenuWidget.hpp"
#include "../GUI/utils.hpp"
#include "../text/TTauriIcons.hpp"
#include "../utils.hpp"
#include <Windows.h>
#include <WinUser.h>
#include <cmath>
#include <typeinfo>

namespace tt {


SystemMenuWidget::SystemMenuWidget(Window &window, Widget *parent, Image const &icon) noexcept :
    Widget(window, parent),
    iconCell(icon.makeCell()),
    systemMenuRectangle(vec{Theme::toolbarDecorationButtonWidth, Theme::toolbarHeight})
{
}

[[nodiscard]] WidgetUpdateResult SystemMenuWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (ttlet result = Widget::updateConstraints(); result < WidgetUpdateResult::Self) {
        return result;
    }

    window.replaceConstraint(minimumWidthConstraint, width >= Theme::toolbarDecorationButtonWidth);
    window.replaceConstraint(maximumWidthConstraint, width <= Theme::toolbarDecorationButtonWidth, rhea::strength::weak());
    window.replaceConstraint(minimumHeightConstraint, height >= Theme::toolbarHeight);
    window.replaceConstraint(maximumHeightConstraint, height <= Theme::toolbarHeight, rhea::strength::weak());
    return WidgetUpdateResult::Self;
}

void SystemMenuWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    iconCell->draw(drawContext, rectangle(), Alignment::MiddleCenter);
    Widget::draw(drawContext, displayTimePoint);
}

HitBox SystemMenuWidget::hitBoxTest(vec position) const noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    if (systemMenuRectangle.contains(position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return HitBox{this, elevation, HitBox::Type::ApplicationIcon};
    } else {
        return {};
    }
}

}
