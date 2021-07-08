// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/// @file

#pragma once

namespace tt {

/** The type of button.
 */
enum class button_type {
    /** A momentary button.
     *
     * A momentary button does not have state. It is used
     * to activate a function each time the button is activated.
     *
     * Example momentary button widgets:
     *  - `momentary_button_widget`
     *  - `toolbar_button_widget`
     */
    momentary,

    /** A toggle button.
     *
     * Activating a toggle button switches the state between 'on' and 'off'.
     * If the state was 'other' activation will switch the state to 'off'.
     *
     * Example toggle button widgets:
     *  - `checkbox_widget`
     *  - `toggle_widget`
     */
    toggle,

    /** A radio button.
     *
     * Activating a radio button will always switch to the 'on' state.
     * Other radio buttons in a set will have different on-values and
     * change the shared state.
     *
     * Example radio button widgets:
     *  - `menu_button_widget`
     *  - `radio_button_widget`
     *  - `toolbar_tab_button_widget`
     */
    radio
};

} // namespace tt
