// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file GFX/sub_pixel_orientation.hpp
 */

#pragma once

#include "../geometry/extent.hpp"

namespace tt::inline v1 {

/** The orientation of the RGB sub-pixels of and LCD/LED panel.
 */
enum class sub_pixel_orientation {
    Unknown = 0,
    BlueRight = 1,
    BlueLeft = 2,
    BlueTop = 3,
    BlueBottom = 4,
};

/** Get the size of a sub-pixel based on the sub-pixel orientation.
 */
[[nodiscard]] constexpr extent2 sub_pixel_size(sub_pixel_orientation orientation) noexcept
{
    switch (orientation) {
    case sub_pixel_orientation::BlueBottom:
    case sub_pixel_orientation::BlueTop: return extent2{1.0f, 1.0f / 3.0f};
    case sub_pixel_orientation::BlueLeft:
    case sub_pixel_orientation::BlueRight: return extent2{1.0f / 3.0f, 1.0f};
    default: return extent2{1.0f, 1.0f};
    }
}

} // namespace tt::inline v1
