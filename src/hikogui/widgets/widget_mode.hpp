// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/widget_mode.hpp Defines widget_mode.
 * @ingroup widget_utilities
 */

#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.widgets.widget_mode);

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

}} // namespace hi::v1
