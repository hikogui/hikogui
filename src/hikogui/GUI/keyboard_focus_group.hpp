// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hi::inline v1 {

/** The keyboard focus group used for finding a widget that will accept a particular focus.
 */
enum class keyboard_focus_group {
    /** A normal widget.
     * Normal widgets accept keyboard focus using tab/shift-tab keys.
     */
    normal = 1,

    /** A menu item in a popup overlay
     * Menu item widget accepts keyboard focus from the up/down cursor keys.
     */
    menu = 2,

    /** A menu item in the toolbar of the window.
     * Menu item widget in the toolbar accepts keyboard focus from the left/right cursor keys
     * and from the main-menu-select key.
     */
    toolbar = 4,

    /** A widget that only accepts keyboard focus from mouse clicks.
     */
    mouse = 8,

    /** Used for selecting any widget that can accept keyboard focus.
     * This is used for selecting a widget of group normal, menu or toolbar using
     * the mouse.
     */
    all = normal | menu | toolbar | mouse,
};

[[nodiscard]] constexpr keyboard_focus_group operator&(keyboard_focus_group const &lhs, keyboard_focus_group const &rhs) noexcept
{
    return static_cast<keyboard_focus_group>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr keyboard_focus_group operator|(keyboard_focus_group const &lhs, keyboard_focus_group const &rhs) noexcept
{
    return static_cast<keyboard_focus_group>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr bool to_bool(keyboard_focus_group group) noexcept
{
    return to_bool(to_underlying(group));
}

} // namespace hi::inline v1
