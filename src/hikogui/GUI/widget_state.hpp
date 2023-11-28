// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../concurrency/concurrency.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <utility>
#include <concepts>
#include <compare>

hi_export_module(hikogui.GUI : widget_state);

hi_export namespace hi { inline namespace v1 {

/** The mode that the widget is operating at.
 *
 * The enumeration here increases visibility and interactivity at each step
 * and you may use `operator<=>()` to compare modes.
 *
 * @ingroup widget_utilities
 */
enum class widget_mode {
    /** The widget is collapsed.
     *
     * This means:
     *  - The widget has zero size and zero margins.
     *  - The widget does not draw itself or its children.
     *  - The widget will not accept any events.
     */
    collapse,

    /** The widget is invisible.
     *
     * This means:
     *  - The widget has size and margins so that it will reserve space in a container.
     */
    invisible,

    /** The widget is disabled.
     *
     * This means:
     *  - The widget "grayed-out"; drawn with less contrast and saturation.
     */
    disabled,

    /** The widget is in display-only mode.
     *
     * This means:
     *  - The widget is drawn normally.
     */
    display,

    /** The widget is selectable.
     *
     * This means:
     *  - The widget or its contents such as text may be selected.
     *  - The widget or its contents may be dragged by the mouse.
     */
    select,

    /** A widget is partially enabled.
     *
     * This means:
     *  - The widget will accept keyboard focus.
     *  - A widget has an extra mode where it limits the amount of control.
     *    such as a text-widget which has a mode where only a single line
     *    can be edited.
     */
    partial,

    /** The widget is fully enabled.
     *
     * This means:
     *  - The widget will accept keyboard focus.
     *  - The widget's state is controllable.
     */
    enabled
};

enum class widget_value {
    off = 0,
    on = 1,
    other = 2,
};

constexpr auto widget_state_mode_shift = 0;
constexpr auto widget_state_value_shift = 5;

// clang-format off
enum class widget_state : uint16_t {
    /** The mode of a widget.
     * 
     * The mode may change the size of the widget and therefor when the value
     * changes a reconstrain is nessesary.
     */
    collapse         = std::to_underlying(widget_mode::collapse) << widget_state_mode_shift,
    invisible        = std::to_underlying(widget_mode::invisible) << widget_state_mode_shift,
    disabled         = std::to_underlying(widget_mode::disabled) << widget_state_mode_shift,
    display          = std::to_underlying(widget_mode::display) << widget_state_mode_shift,
    select           = std::to_underlying(widget_mode::select) << widget_state_mode_shift,
    partial          = std::to_underlying(widget_mode::partial) << widget_state_mode_shift,
    enabled          = std::to_underlying(widget_mode::enabled) << widget_state_mode_shift,
    mode_mask        = 0b111 << widget_state_mode_shift,

    /** The nesting layer for widgets.
     * 
     * The layer mostly determines the background color of nested widgets.
     */
    layer0           = 0 << 3,
    layer1           = 1 << 3,
    layer2           = 2 << 3,
    layer3           = 3 << 3,

    /** The value that the widget represents.
     * 
     * The value may cause a change of color or draw shape to change.
    */
    off              = std::to_underlying(widget_value::off) << widget_state_value_shift,
    on               = std::to_underlying(widget_value::on) << widget_state_value_shift,
    other            = std::to_underlying(widget_value::other) << widget_state_value_shift,
    value_mask       = 0b11 << widget_state_value_shift,

    /** Keyboard focus
     */
    focus            = 1 << 7,

    /** Mouse hover
     */
    hover            = 1 << 8,

    /** Mouse clicked
     */
    pressed          = 1 << 9,

    /** Window active
     */
    active           = 1 << 10,

    reconstrain_mask = 0b0000'00'00'111,
    relayout_mask    = 0b0000'00'00'111,
    redraw_mask      = 0b1111'11'11'111,
    notify_mask      = 0b0000'11'00'000,
};
// clang-format on

[[nodiscard]] constexpr widget_state operator|(widget_state const &lhs, widget_state const &rhs) noexcept
{
    return static_cast<widget_state>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr widget_state operator&(widget_state const &lhs, widget_state const &rhs) noexcept
{
    return static_cast<widget_state>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

[[nodiscard]] constexpr widget_state operator^(widget_state const &lhs, widget_state const &rhs) noexcept
{
    return static_cast<widget_state>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
}

[[nodiscard]] constexpr widget_state operator~(widget_state const &rhs) noexcept
{
    return static_cast<widget_state>(~std::to_underlying(rhs));
}

constexpr widget_state &operator|=(widget_state &lhs, widget_state const &rhs) noexcept
{
    return lhs = lhs | rhs;
}

constexpr widget_state &operator&=(widget_state &lhs, widget_state const &rhs) noexcept
{
    return lhs = lhs & rhs;
}

constexpr widget_state &operator^=(widget_state &lhs, widget_state const &rhs) noexcept
{
    return lhs = lhs ^ rhs;
}

[[nodiscard]] constexpr bool to_bool(widget_state const &rhs) noexcept
{
    return std::to_underlying(rhs) != 0;
}

[[nodiscard]] constexpr bool need_reconstrain(widget_state const &old_state, widget_state const &new_state) noexcept
{
    return to_bool((old_state ^ new_state) & widget_state::reconstrain_mask);
} 

[[nodiscard]] constexpr bool need_relayout(widget_state const &old_state, widget_state const &new_state) noexcept
{
    return to_bool((old_state ^ new_state) & widget_state::relayout_mask);
} 

[[nodiscard]] constexpr bool need_redraw(widget_state const &old_state, widget_state const &new_state) noexcept
{
    return to_bool((old_state ^ new_state) & widget_state::redraw_mask);
} 

[[nodiscard]] constexpr bool need_notify(widget_state const &old_state, widget_state const &new_state) noexcept
{
    return to_bool((old_state ^ new_state) & widget_state::notify_mask);
} 

[[nodiscard]] constexpr size_t to_layer(widget_state const &rhs) noexcept
{
    return (std::to_underlying(rhs) >> 3) & 0b11;
}

[[nodiscard]] constexpr widget_mode to_mode(widget_state const &rhs) noexcept
{
    return static_cast<widget_mode>(std::to_underlying(rhs) & 0b111);
}

[[nodiscard]] constexpr widget_state set_mode(widget_state const &lhs, widget_mode const &rhs) noexcept
{
    return (lhs & ~widget_state::mode_mask) | static_cast<widget_state>(std::to_underlying(rhs) << widget_state_mode_shift);
}

[[nodiscard]] constexpr widget_value to_value(widget_state const &rhs) noexcept
{
    return static_cast<widget_value>((std::to_underlying(rhs) >> 5) & 0b11);
}

[[nodiscard]] constexpr widget_state set_value(widget_state const &lhs, widget_value const &rhs) noexcept
{
    return (lhs & ~widget_state::value_mask) | static_cast<widget_state>(std::to_underlying(rhs) << widget_state_value_shift);
}


}}
