// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "system_menu_widget.hpp"

namespace tt {

system_menu_widget::system_menu_widget(gui_window &window, widget *parent) noexcept :
    super(window, parent)
{
}

void system_menu_widget::init() noexcept
{
    _icon_widget = &make_widget<icon_widget>(icon);
}

[[nodiscard]] float system_menu_widget::margin() const noexcept
{
    return 0.0f;
}

[[nodiscard]] bool
system_menu_widget::constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::constrain(display_time_point, need_reconstrain)) {
        ttlet width = theme().toolbar_decoration_button_width;
        ttlet height = theme().toolbar_height;
        _minimum_size = _preferred_size = _maximum_size = {width, height};
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void system_menu_widget::layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _request_layout.exchange(false);
    if (need_layout) {
        ttlet icon_height =
            rectangle().height() < theme().toolbar_height * 1.2f ? rectangle().height() : theme().toolbar_height;
        ttlet icon_rectangle = aarectangle{rectangle().left(), rectangle().top() - icon_height, rectangle().width(), icon_height};

        _icon_widget->set_layout_parameters_from_parent(icon_rectangle);

        // Leave space for window resize handles on the left and top.
        system_menu_rectangle = aarectangle{
            rectangle().left() + theme().margin,
            rectangle().bottom(),
            rectangle().width() - theme().margin,
            rectangle().height() - theme().margin};
    }

    super::layout(display_time_point, need_layout);
}

hitbox system_menu_widget::hitbox_test(point2 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (_visible_rectangle.contains(position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return hitbox{this, draw_layer, hitbox::Type::ApplicationIcon};
    } else {
        return {};
    }
}

} // namespace tt
