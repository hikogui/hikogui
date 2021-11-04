// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace tt {
inline namespace v1 {

/** The keyboard focus group used for finding a widget that will accept a particular focus.
 */
enum class keyboard_focus_group {
    /** A normal widget.
     * Normal widgets accept keyboard focus using tab/shift-tab keys.
     */
    normal,

    /** A menu item in a popup overlay
     * Menu item widget accepts keyboard focus from the up/down cursor keys.
     */
    menu,

    /** A menu item in the toolbar of the window.
     * Menu item widget in the toolbar accepts keyboard focus from the left/right cursor keys
     * and from the main-menu-select key.
     */
    toolbar,

    /** Used for selecting any widget that can accept keyboard focus.
     * This is used for selecting a widget of group normal, menu or toolbar using
     * the mouse.
     */
    any,
};

[[nodiscard]] constexpr bool is_normal(keyboard_focus_group group) noexcept
{
    using enum keyboard_focus_group;
    return group == normal || group == any;
}

[[nodiscard]] constexpr bool is_menu(keyboard_focus_group group) noexcept
{
    using enum keyboard_focus_group;
    return group == menu || group == any;
}

[[nodiscard]] constexpr bool is_toolbar(keyboard_focus_group group) noexcept
{
    using enum keyboard_focus_group;
    return group == toolbar || group == any;
}

}
}
