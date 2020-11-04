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

SystemMenuWidget::SystemMenuWidget(Window &window, std::shared_ptr<widget> parent, icon const &icon) noexcept :
    widget(window, parent), _icon_stencil(stencil::make_unique(Alignment::MiddleCenter, icon))
{
    // Toolbar buttons hug the toolbar and neighbour widgets.
    _margin = 0.0f;
}

[[nodiscard]] bool SystemMenuWidget::update_constraints() noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    if (widget::update_constraints()) {
        ttlet width = Theme::toolbarDecorationButtonWidth;
        ttlet height = Theme::toolbarHeight;
        _preferred_size = {vec{width, height}, vec{width, std::numeric_limits<float>::infinity()}};
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] bool SystemMenuWidget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    need_layout |= std::exchange(_request_relayout, false);
    if (need_layout) {
        ttlet icon_height = rectangle().height() < Theme::toolbarHeight * 1.2f ? rectangle().height() : Theme::toolbarHeight;
        ttlet icon_rectangle = aarect{
            rectangle().x(),
            rectangle().top() - icon_height,
            rectangle().width(),
            icon_height
        };

        _icon_stencil->set_layout_parameters(icon_rectangle);

        // Leave space for window resize handles on the left and top.
        system_menu_rectangle = aarect{
            rectangle().x() + Theme::margin,
            rectangle().y(),
            rectangle().width() - Theme::margin,
            rectangle().height() - Theme::margin};
    }

    return widget::update_layout(display_time_point, need_layout);
}

void SystemMenuWidget::draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    _icon_stencil->draw(context);
    widget::draw(std::move(context), display_time_point);
}

HitBox SystemMenuWidget::hitbox_test(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    if (_window_clipping_rectangle.contains(window_position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return HitBox{weak_from_this(), _draw_layer, HitBox::Type::ApplicationIcon};
    } else {
        return {};
    }
}

} // namespace tt
