// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

namespace tt {

/** The keyboard focus group used for finding a widget that will accept a particular focus.
 */
enum class keyboard_focus_direction {
    /** Search backward in the keyboard focus chain.
     */
    backward,

    /** Search forward in the keyboard focus chain.
     */
    forward,

    /** The current widget.
     */
    current
};

} // namespace tt