// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/system_menu_widget.hpp Defines system_menu_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "icon_widget.hpp"
#include "../l10n/l10n.hpp"
#include "../macros.hpp"
#include <memory>
#include <string>
#include <array>
#include <coroutine>

hi_export_module(hikogui.widgets.system_menu_widget);

hi_export namespace hi {
inline namespace v1 {

/** The system menu widget.
 * This widget displays an icon in the menu bar of the window and is used to call-up
 * the operating-system supplied menu to control the window.
 *
 * @ingroup widgets
 */
class system_menu_widget : public widget {
public:
    using super = widget;

    observer<icon> icon;

    ~system_menu_widget() {}

    system_menu_widget() noexcept : super()
    {
        _icon_widget = std::make_unique<icon_widget>(icon);
        _icon_widget->set_parent(this);

        style.set_name("system-menu");
    }

    template<forward_of<observer<hi::icon>> Icon>
    system_menu_widget(Icon&& icon) noexcept : system_menu_widget()
    {
        this->icon = std::forward<Icon>(icon);
    }

    /// @privatesection
    [[nodiscard]] generator<widget_intf&> children(bool include_invisible) const noexcept override
    {
        co_yield *_icon_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        hi_assert_not_null(_icon_widget);

        _icon_constraints = _icon_widget->update_constraints();

        return {
            style.size_px,
            style.size_px,
            style.size_px,
            style.margins_px,
            baseline::from_middle_of_object(style.baseline_priority, style.cap_height_px, style.height_px)};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        super::set_layout(context);

        _icon_shape = box_shape{context.rectangle(), context.baseline()};
        // Leave space for window resize handles on the left and top.
        _system_menu_rectangle = aarectangle{
            point2{context.left() + style.margin_left_px, context.bottom()},
            point2{context.right(), context.top() - style.margin_top_px}};

        _icon_widget->set_layout(context.transform(_icon_shape));
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (enabled() and layout().contains(position)) {
            // Only the top-left square should return ApplicationIcon, leave
            // the reset to the toolbar implementation.
            return {id(), layout().elevation, hitbox_type::application_icon};
        } else {
            return {};
        }
    }

    /// @endprivatesection
private:
    std::unique_ptr<icon_widget> _icon_widget;
    box_constraints _icon_constraints;
    box_shape _icon_shape;

    aarectangle _system_menu_rectangle;
};

} // namespace v1
} // namespace hi::v1
