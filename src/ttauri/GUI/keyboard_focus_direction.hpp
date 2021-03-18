// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

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
};

} // namespace tt