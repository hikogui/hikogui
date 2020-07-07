// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/widgets/SystemMenuWidget.hpp"
#include "ttauri/GUI/utils.hpp"
#include "ttauri/text/TTauriIcons.hpp"
#include "ttauri/utils.hpp"
#include <Windows.h>
#include <WinUser.h>
#include <cmath>
#include <typeinfo>

namespace tt {


SystemMenuWidget::SystemMenuWidget(Window &window, Widget *parent, Image const &icon) noexcept :
    Widget(window, parent, vec{Theme::toolbarDecorationButtonWidth, Theme::toolbarHeight}),
    iconCell(icon.makeCell()),
    systemMenuRectangle(vec{Theme::toolbarDecorationButtonWidth, Theme::toolbarHeight})
{
    setFixedExtent(vec{Theme::toolbarDecorationButtonWidth, Theme::toolbarHeight});
}


void SystemMenuWidget::layout(hires_utc_clock::time_point displayTimePoint) noexcept
{
    Widget::layout(displayTimePoint);
}

void SystemMenuWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    iconCell->draw(drawContext, rectangle(), Alignment::MiddleCenter);
    Widget::draw(drawContext, displayTimePoint);
}

HitBox SystemMenuWidget::hitBoxTest(vec position) const noexcept
{
    if (systemMenuRectangle.contains(position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return HitBox{this, elevation, HitBox::Type::ApplicationIcon};
    } else {
        return {};
    }
}

}
