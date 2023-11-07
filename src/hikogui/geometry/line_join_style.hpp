// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file geometry/line_join_style.hpp Defines line_join_style
 * @ingroup geometry
 */

#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.geometry : line_join_style);

hi_export namespace hi {
inline namespace v1 {

/** The way two lines should be joined.
 * @ingroup geometry
 */
enum class line_join_style
{
    /** Both lines have flat caps and are not joined.
     */
    none,

    /** The outer vertices of both lines are connected and filled in to make a blunt corner.
     */
    bevel,

    /** The outer edge of both lines are extended until they meet to form a sharp corner.
    */
    miter,

    /** Both lines have round caps and with a shared mid-point making a round corner.
     */
    round
};

}}

