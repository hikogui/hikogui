// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "system_menu_widget.hpp"

namespace tt {

system_menu_widget::system_menu_widget(gui_window &window, widget *parent) noexcept :
    super(window, parent)
{
    _icon_widget = std::make_unique<icon_widget>(window, this, icon);
}

[[nodiscard]] float system_menu_widget::margin() const noexcept
{
    return 0.0f;
}

[[nodiscard]] bool
system_menu_widget::constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept
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

void system_menu_widget::layout(matrix3 const &to_window, extent2 const &new_size, utc_nanoseconds display_time_point, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    if (set_layout(to_window, new_size) or need_layout) {
        ttlet icon_height =
            rectangle().height() < theme().toolbar_height * 1.2f ? rectangle().height() : theme().toolbar_height;
        ttlet icon_rectangle = aarectangle{rectangle().left(), rectangle().top() - icon_height, rectangle().width(), icon_height};

        _icon_widget->set_layout_parameters_from_parent(icon_rectangle);
        _icon_widget->layout(translate2{icon_rectangle} * to_window, icon_rectangle.size(), display_time_point, need_layout);

        // Leave space for window resize handles on the left and top.
        system_menu_rectangle = aarectangle{
            rectangle().left() + theme().margin,
            rectangle().bottom(),
            rectangle().width() - theme().margin,
            rectangle().height() - theme().margin};
        request_redraw();
    }
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
