// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/system_menu_widget.hpp Defines system_menu_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
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
class system_menu_widget final : public widget {
public:
    using super = widget;

    observer<icon> icon;

    ~system_menu_widget() {}

    system_menu_widget(gui_window& window, widget *parent) noexcept;

    system_menu_widget(gui_window& window, widget *parent, forward_of<observer<hi::icon>> auto&& icon) noexcept :
        system_menu_widget(window, parent)
    {
        this->icon = hi_forward(icon);
    }

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override
    {
        co_yield _icon_widget.get();
    }

    widget_constraints const& set_constraints() noexcept override;
    void set_layout(widget_layout const& layout) noexcept override;
    void draw(draw_context const& context) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override;
    /// @endprivatesection
private:
    aarectangle _icon_rectangle;
    std::unique_ptr<icon_widget> _icon_widget;

    aarectangle _system_menu_rectangle;
};

}} // namespace hi::v1
