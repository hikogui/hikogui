// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "system_menu_widget.hpp"

namespace hi::inline v1 {

system_menu_widget::system_menu_widget(widget *parent) noexcept : super(parent)
{
    _icon_widget = std::make_shared<icon_widget>(this, icon);
}

box_constraints const& system_menu_widget::get_constraints(get_constraints_context const& context) noexcept
{
    _layout = {};
    _icon_constraints = _icon_widget->get_constraints(context);

    hilet size = extent2{context.theme->large_size, context.theme->large_size};
    return {size, size, size};
}

void system_menu_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet icon_height = context.height() < context.theme->large_size * 1.2f ? context.height() : context.theme->large_size;
        hilet icon_rectangle = aarectangle{0.0f, narrow_cast<float>(context.height()) - icon_height, narrow_cast<float>(context.width()), icon_height};
        _icon_shape = box_shape{_icon_constraints, icon_rectangle, context.theme->baseline_adjustment};
        // Leave space for window resize handles on the left and top.
        _system_menu_rectangle = aarectangle{
            context.theme->margin, 0.0f, narrow_cast<float>(context.width()) - context.theme->margin, narrow_cast<float>(context.height()) - context.theme->margin};
    }

    _icon_widget->set_layout(context.transform(_icon_shape));
}

void system_menu_widget::draw(draw_context const &context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        _icon_widget->draw(context);
    }
}

hitbox system_menu_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    if (*mode >= widget_mode::partial and layout().contains(position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return {this, position, hitbox_type::application_icon};
    } else {
        return {};
    }
}

} // namespace hi::inline v1
