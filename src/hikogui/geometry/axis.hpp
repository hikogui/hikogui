// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file axis.hpp The axis data type.
 */

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <compare>

hi_export_module(hikogui.geometry : axis);

hi_export namespace hi {
inline namespace v1 {

/** An enumeration of the 3 axis for 3D geometry.
 * @ingroup geometry
 *
 * It can be used as flags/mask for a set of axis.
 */
enum class axis : unsigned char {
    none = 0,
    x = 1,
    y = 2,
    z = 4,
    both = x | y,
    all = x | y | z,

    horizontal = x,
    vertical = y,
};

/** AND the axis 
 * @ingroup geometry
 */
[[nodiscard]] constexpr axis operator&(axis const &lhs, axis const &rhs) noexcept
{
    return static_cast<axis>(static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs));
}

/** OR the axis 
 * @ingroup geometry
 */
[[nodiscard]] constexpr axis operator|(axis const &lhs, axis const &rhs) noexcept
{
    return static_cast<axis>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
}

/** Check if any of the axis are set.
 * @ingroup geometry
 */
[[nodiscard]] constexpr bool to_bool(axis const& rhs) noexcept
{
    return to_bool(static_cast<unsigned char>(rhs));
}

}} // namespace hi::inline v1
