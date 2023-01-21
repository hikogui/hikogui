// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "system_menu_widget.hpp"

namespace hi::inline v1 {

system_menu_widget::system_menu_widget(widget *parent) noexcept : super(parent)
{
    _icon_widget = std::make_unique<icon_widget>(this, icon);
}

[[nodiscard]] box_constraints system_menu_widget::update_constraints() noexcept
{
    hi_assert_not_null(_icon_widget);

    _layout = {};
    _icon_constraints = _icon_widget->update_constraints();

    hilet size = extent2i{theme().large_size(), theme().large_size()};
    return {size, size, size};
}

void system_menu_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet icon_height =
            context.height() < round_cast<int>(theme().large_size() * 1.2f) ? context.height() : theme().large_size();
        hilet icon_rectangle = aarectanglei{0, context.height() - icon_height, context.width(), icon_height};
        _icon_shape = box_shape{_icon_constraints, icon_rectangle, theme().baseline_adjustment()};
        // Leave space for window resize handles on the left and top.
        _system_menu_rectangle = aarectanglei{
            theme().margin<int>(), 0, context.width() - theme().margin<int>(), context.height() - theme().margin<int>()};
    }

    _icon_widget->set_layout(context.transform(_icon_shape));
}

void system_menu_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        _icon_widget->draw(context);
    }
}

hitbox system_menu_widget::hitbox_test(point2i position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    if (*mode >= widget_mode::partial and layout().contains(position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return {id, _layout.elevation, hitbox_type::application_icon};
    } else {
        return {};
    }
}

} // namespace hi::inline v1
