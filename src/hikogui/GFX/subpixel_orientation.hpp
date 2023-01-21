// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file GFX/subpixel_orientation.hpp
 */

#pragma once

#include "../geometry/module.hpp"
#include "../utility/module.hpp"
#include <format>

namespace hi::inline v1 {

/** The orientation of the RGB sub-pixels of and LCD/LED panel.
 */
enum class subpixel_orientation {
    unknown,
    horizontal_rgb,
    horizontal_bgr,
    vertical_rgb,
    vertical_bgr,
};

// clang-format off
constexpr auto subpixel_orientation_metadata = enum_metadata{
    subpixel_orientation::unknown, "unknown",
    subpixel_orientation::horizontal_rgb, "horizontal RGB",
    subpixel_orientation::horizontal_bgr, "horizontal BGR",
    subpixel_orientation::vertical_rgb, "vertical RGB",
    subpixel_orientation::vertical_bgr, "vertical BGR",
};
// clang-format on

/** Get the size of a sub-pixel based on the sub-pixel orientation.
 */
[[nodiscard]] constexpr extent2 sub_pixel_size(subpixel_orientation orientation) noexcept
{
    switch (orientation) {
    case subpixel_orientation::vertical_rgb:
    case subpixel_orientation::vertical_bgr: return extent2{1.0f, 1.0f / 3.0f};
    case subpixel_orientation::horizontal_bgr:
    case subpixel_orientation::horizontal_rgb: return extent2{1.0f / 3.0f, 1.0f};
    default: return extent2{1.0f, 1.0f};
    }
}

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::subpixel_orientation, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::subpixel_orientation const &t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(hi::subpixel_orientation_metadata[t], fc);
    }
};
