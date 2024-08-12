// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../concurrency/concurrency.hpp"
#include "../theme/theme.hpp"
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
    collapse = 0,

    /** The widget is invisible.
     *
     * This means:
     *  - The widget has size and margins so that it will reserve space in a container.
     */
    invisible = 1,

    /** The widget is disabled.
     *
     * This means:
     *  - The widget "grayed-out"; drawn with less contrast and saturation.
     */
    disabled = 2,

    /** The widget is in display-only mode.
     *
     * This means:
     *  - The widget is drawn normally.
     */
    display = 3,

    /** The widget is selectable.
     *
     * This means:
     *  - The widget or its contents such as text may be selected.
     *  - The widget or its contents may be dragged by the mouse.
     */
    select = 4,

    /** A widget is partially enabled.
     *
     * This means:
     *  - The widget will accept keyboard focus.
     *  - A widget has an extra mode where it limits the amount of control.
     *    such as a text-widget which has a mode where only a single line
     *    can be edited.
     */
    partial = 5,

    /** The widget is fully enabled.
     *
     * This means:
     *  - The widget will accept keyboard focus.
     *  - The widget's state is controllable.
     */
    enabled = 6,
};

enum class widget_value {
    off = 0,
    on = 1,
    other = 2,
};

enum class widget_phase {
    /** The widget is disabled and shown with a muted color.
     */
    disabled = 0,

    /** The widget is enabled.
     */
    enabled = 1,

    /** The mouse is hovering over the widget.
     */
    hover = 2,

    /** The widget is being activated by a click or keyboard action.
     */
    active = 3,
};

constexpr auto widget_state_mode_shift = 0;
constexpr auto widget_state_value_shift = 5;


}}
