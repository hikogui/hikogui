// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/// @file

#pragma once

namespace tt::inline v1 {

/** The state of a button.
 */
enum class button_state {
    /** The 'off' state of a button.
     */
    off,

    /** The 'on' state of a button.
     */
    on,

    /** The other state of a button.
     *
     * For checkboxes the 'other' state is when the value it represents is
     * neither 'on' or 'off'. Examples off this is when the checkbox is a parent
     * in a tree structure, where 'other' represents that its children have
     * different values.
     */
    other
};

} // namespace tt::inline v1
