// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file image/sfloat_rgb32.hpp Defines the sfloat_rgb32.
 * @ingroup image
 */

#pragma once

#include "../SIMD/SIMD.hpp"
#include "../geometry/geometry.hpp"
#include "../macros.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>

hi_export_module(hikogui.image.sfloat_rgb32);

hi_export namespace hi::inline v1 {

/** 3 x float32 pixel format.
 *
 * @ingroup image
 */
class sfloat_rgb32 {
    // Red, Green, Blue in binary32 (native endian).
    std::array<float, 3> v;

public:
    sfloat_rgb32() = default;
    sfloat_rgb32(sfloat_rgb32 const &rhs) noexcept = default;
    sfloat_rgb32(sfloat_rgb32 &&rhs) noexcept = default;
    sfloat_rgb32 &operator=(sfloat_rgb32 const &rhs) noexcept = default;
    sfloat_rgb32 &operator=(sfloat_rgb32 &&rhs) noexcept = default;

    sfloat_rgb32(f32x4 const &rhs) noexcept : v{rhs.r(), rhs.g(), rhs.b()} {}

    sfloat_rgb32 &operator=(f32x4 const &rhs) noexcept
    {
        v = {rhs.r(), rhs.g(), rhs.b()};
        return *this;
    }

    operator f32x4() const noexcept
    {
        return f32x4{std::get<0>(v), std::get<1>(v), std::get<2>(v), 0.0f};
    }

    sfloat_rgb32(point3 const &rhs) noexcept : sfloat_rgb32(static_cast<f32x4>(rhs)) {}

    sfloat_rgb32 &operator=(point3 const &rhs) noexcept
    {
        return *this = static_cast<f32x4>(rhs);
    }

    operator point3() const noexcept
    {
        return point3{f32x4{*this}};
    }

    [[nodiscard]] friend bool operator==(sfloat_rgb32 const &lhs, sfloat_rgb32 const &rhs) noexcept = default;
};

} // namespace hi::inline v1
