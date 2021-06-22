// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "system_menu_widget.hpp"
#include "../GUI/utils.hpp"
#include "../text/ttauri_icon.hpp"
#include "../utils.hpp"
#include <Windows.h>
#include <WinUser.h>
#include <cmath>
#include <typeinfo>

namespace tt {

system_menu_widget::system_menu_widget(gui_window &window, std::shared_ptr<widget> parent) noexcept :
    super(window, std::move(parent))
{
    // Toolbar buttons hug the toolbar and neighbor widgets.
    _margin = 0.0f;
}

void system_menu_widget::init() noexcept
{
    _icon_widget = make_widget<icon_widget>(icon);
}

[[nodiscard]] bool
system_menu_widget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::update_constraints(display_time_point, need_reconstrain)) {
        ttlet width = theme::global().toolbar_decoration_button_width;
        ttlet height = theme::global().toolbar_height;
        _minimum_size = _preferred_size = _maximum_size = {width, height};
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void system_menu_widget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= std::exchange(_request_relayout, false);
    if (need_layout) {
        ttlet icon_height =
            rectangle().height() < theme::global().toolbar_height * 1.2f ? rectangle().height() : theme::global().toolbar_height;
        ttlet icon_rectangle = aarectangle{rectangle().left(), rectangle().top() - icon_height, rectangle().width(), icon_height};

        _icon_widget->set_layout_parameters_from_parent(icon_rectangle);

        // Leave space for window resize handles on the left and top.
        system_menu_rectangle = aarectangle{
            rectangle().left() + theme::global().margin,
            rectangle().bottom(),
            rectangle().width() - theme::global().margin,
            rectangle().height() - theme::global().margin};
    }

    super::update_layout(display_time_point, need_layout);
}

hit_box system_menu_widget::hitbox_test(point2 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (_visible_rectangle.contains(position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return hit_box{weak_from_this(), _draw_layer, hit_box::Type::ApplicationIcon};
    } else {
        return {};
    }
}

} // namespace tt
