// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file geometry/line_end_cap.hpp Defined line_end_cap.
 * @ingroup geometry
 */

#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.geometry : line_end_cap);

hi_export namespace hi {
inline namespace v1 {

/** The way two lines should be joined.
 * @ingroup geometry
 */
enum class line_end_cap {
    /** The end cap of the line is flat.
     */
    flat,

    /** The end cap of the line is round.
     */
    round
};

}} // namespace hi::inline v1

