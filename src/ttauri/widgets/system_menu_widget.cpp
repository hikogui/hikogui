// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "system_menu_widget.hpp"

namespace tt {

system_menu_widget::system_menu_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    _icon_widget = std::make_unique<icon_widget>(window, this, icon);
}

[[nodiscard]] float system_menu_widget::margin() const noexcept
{
    return 0.0f;
}

void system_menu_widget::constrain() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};
    _icon_widget->constrain();

    ttlet width = theme().toolbar_decoration_button_width;
    ttlet height = theme().toolbar_height;
    _minimum_size = _preferred_size = _maximum_size = {width, height};
    tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
}

void system_menu_widget::layout(layout_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        if (compare_then_assign(_layout, context)) {
            request_redraw();

            ttlet icon_height =
                rectangle().height() < theme().toolbar_height * 1.2f ? rectangle().height() : theme().toolbar_height;
            _icon_rectangle = aarectangle{rectangle().left(), rectangle().top() - icon_height, rectangle().width(), icon_height};

            // Leave space for window resize handles on the left and top.
            _system_menu_rectangle = aarectangle{
                rectangle().left() + theme().margin,
                rectangle().bottom(),
                rectangle().width() - theme().margin,
                rectangle().height() - theme().margin};
        }

        _icon_widget->layout(_icon_rectangle * context);
    }
}

void system_menu_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, _layout)) {
        _icon_widget->draw(context);
    }
}

hitbox system_menu_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (_layout.hit_rectangle.contains(position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return hitbox{this, position, hitbox::Type::ApplicationIcon};
    } else {
        return {};
    }
}

} // namespace tt
