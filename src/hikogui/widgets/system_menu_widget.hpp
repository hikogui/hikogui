// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/system_menu_widget.hpp Defines system_menu_widget.
 * @ingroup widgets
 */

#pragma once

#include "../GUI/module.hpp"
#include "icon_widget.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>

namespace hi { inline namespace v1 {

/** The system menu widget.
 * This widget displays an icon in the menu bar of the window and is used to call-up
 * the operating-system supplied menu to control the window.
 *
 * @ingroup widgets
 */
template<fixed_string Name = "">
class system_menu_widget final : public widget {
public:
    using super = widget;
    constexpr static auto prefix = Name / "system-menu";

    observer<icon> icon;

    ~system_menu_widget() {}

    system_menu_widget(widget *parent) noexcept : super(parent)
    {
        _icon_widget = std::make_unique<icon_widget<prefix>>(this, icon);
    }

    system_menu_widget(widget *parent, forward_of<observer<hi::icon>> auto&& icon) noexcept : system_menu_widget(parent)
    {
        this->icon = hi_forward(icon);
    }

    /// @privatesection
    [[nodiscard]] generator<widget const&> children(bool include_invisible) const noexcept override
    {
        co_yield *_icon_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        hi_assert_not_null(_icon_widget);

        _icon_constraints = _icon_widget->update_constraints();

        hilet size = theme<prefix>.size(this);
        return {size, size, size};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(layout, context)) {
            hilet size = theme<prefix>.size(this);
            hilet margin = theme<prefix>.margin(this);

            hilet icon_height = context.height() < round_cast<int>(size.height() * 1.2f) ? context.height() : size.height();
            hilet icon_rectangle = aarectanglei{0, context.height() - icon_height, context.width(), icon_height};
            _icon_shape = box_shape{_icon_constraints, icon_rectangle, theme<prefix>.cap_height(this)};
            // Leave space for window resize handles on the left and top.
            _system_menu_rectangle =
                aarectanglei{margin.left(), 0, context.width() - margin.right(), context.height() - margin.top()};
        }

        _icon_widget->set_layout(context.transform(_icon_shape));
    }

    void draw(widget_draw_context& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout)) {
            _icon_widget->draw(context);
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2i position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout.contains(position)) {
            // Only the top-left square should return ApplicationIcon, leave
            // the reset to the toolbar implementation.
            return {id, layout.elevation, hitbox_type::application_icon};
        } else {
            return {};
        }
    }
    /// @endprivatesection
private:
    std::unique_ptr<icon_widget<prefix>> _icon_widget;
    box_constraints _icon_constraints;
    box_shape _icon_shape;

    aarectanglei _system_menu_rectangle;
};

}} // namespace hi::v1
