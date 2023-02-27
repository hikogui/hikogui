// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/widget_state.hpp Defines widget_state.
 * @ingroup widget_utilities
 */

#pragma once

namespace hi {
inline namespace v1 {

enum class widget_state {
    /** The widget in the off-state.
    *
    * i.e. Radio button or checkbox is not-checked.
    */
    off,

    /** The widget is in the on-state.
     *
     * i.e. Radio button or checkbox is checked.
     */
    on,

    /** The widget is in the other-state.
     *
     * i.e. Checkbox shows a partial match.
     */
    other
};

}}
