// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "system_menu_widget.hpp"

namespace hi::inline v1 {

system_menu_widget::system_menu_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    _icon_widget = std::make_unique<icon_widget>(window, this, icon);
}

widget_constraints const &system_menu_widget::set_constraints() noexcept
{
    _layout = {};
    _icon_widget->set_constraints();

    hilet size = extent2{theme().large_size, theme().large_size};
    return _constraints = {size, size, size};
}

void system_menu_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        hilet icon_height = layout.height() < theme().large_size * 1.2f ? layout.height() : theme().large_size;
        _icon_rectangle = aarectangle{0.0f, layout.height() - icon_height, layout.width(), icon_height};

        // Leave space for window resize handles on the left and top.
        _system_menu_rectangle =
            aarectangle{theme().margin, 0.0f, layout.width() - theme().margin, layout.height() - theme().margin};
    }

    _icon_widget->set_layout(layout.transform(_icon_rectangle));
}

void system_menu_widget::draw(draw_context const &context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        _icon_widget->draw(context);
    }
}

hitbox system_menu_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(is_gui_thread());

    if (*mode >= widget_mode::partial and layout().contains(position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return {this, position, hitbox::Type::ApplicationIcon};
    } else {
        return {};
    }
}

} // namespace hi::inline v1
