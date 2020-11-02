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

SystemMenuWidget::SystemMenuWidget(Window &window, std::shared_ptr<Widget> parent, icon const &icon) noexcept :
    Widget(window, parent), iconCell(icon.makeCell())
{
    // Toolbar buttons hug the toolbar and neighbour widgets.
    p_margin = 0.0f;
}

[[nodiscard]] bool SystemMenuWidget::update_constraints() noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    if (Widget::update_constraints()) {
        ttlet width = Theme::toolbarDecorationButtonWidth;
        ttlet height = Theme::toolbarHeight;
        p_preferred_size = {vec{width, height}, vec{width, std::numeric_limits<float>::infinity()}};
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] bool SystemMenuWidget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    need_layout |= std::exchange(request_relayout, false);
    if (need_layout) {
        // Leave space for window resize handles on the left and top.
        system_menu_rectangle = aarect{
            rectangle().x() + Theme::margin,
            rectangle().y(),
            rectangle().width() - Theme::margin,
            rectangle().height() - Theme::margin};
    }

    return Widget::update_layout(display_time_point, need_layout);
}

void SystemMenuWidget::draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    iconCell->draw(context, rectangle(), Alignment::MiddleCenter);
    Widget::draw(std::move(context), display_time_point);
}

HitBox SystemMenuWidget::hitbox_test(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    if (p_window_clipping_rectangle.contains(window_position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return HitBox{weak_from_this(), p_draw_layer, HitBox::Type::ApplicationIcon};
    } else {
        return {};
    }
}

} // namespace tt
