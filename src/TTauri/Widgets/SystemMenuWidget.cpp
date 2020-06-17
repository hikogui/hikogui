// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Widgets/SystemMenuWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Text/TTauriIcons.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <Windows.h>
#include <WinUser.h>
#include <cmath>
#include <typeinfo>

namespace tt {


SystemMenuWidget::SystemMenuWidget(Window &window, Widget *parent, Image icon) noexcept :
    Widget(window, parent, vec{Theme::toolbarDecorationButtonWidth, Theme::toolbarHeight}),
    icon(std::move(icon)),
    systemMenuRectangle(vec{Theme::toolbarDecorationButtonWidth, Theme::toolbarHeight})
{
    setFixedExtent(vec{Theme::toolbarDecorationButtonWidth, Theme::toolbarHeight});
}


void SystemMenuWidget::layout(hires_utc_clock::time_point displayTimePoint) noexcept
{
    Widget::layout(displayTimePoint);

    icon.prepareForDrawing(window);
}

void SystemMenuWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    Widget::draw(drawContext, displayTimePoint);

    if (icon.draw(drawContext, rectangle())) {
        forceRedraw = true;
    }
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
