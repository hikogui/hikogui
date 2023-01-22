// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file image/sfloat_rg32.hpp Defines the sfloat_rg32.
 * @ingroup image
 */

#pragma once

#include "../SIMD/module.hpp"
#include "../geometry/module.hpp"
#include <algorithm>

namespace hi::inline v1 {

/** 2 x float32 pixel format.
* 
* @ingroup image
*/
class sfloat_rg32 {
    alignas(sizeof(float) * 2)
        // Red, Green in binary32 (native endian).
        std::array<float, 2> v;

public:
    sfloat_rg32() = default;
    sfloat_rg32(sfloat_rg32 const &rhs) noexcept = default;
    sfloat_rg32(sfloat_rg32 &&rhs) noexcept = default;
    sfloat_rg32 &operator=(sfloat_rg32 const &rhs) noexcept = default;
    sfloat_rg32 &operator=(sfloat_rg32 &&rhs) noexcept = default;

    explicit sfloat_rg32(f32x4 const &rhs) noexcept : v{rhs.r(), rhs.g()} {}

    sfloat_rg32 &operator=(f32x4 const &rhs) noexcept
    {
        v = {rhs.r(), rhs.g()};
        return *this;
    }

    explicit operator f32x4() const noexcept
    {
        return f32x4{std::get<0>(v), std::get<1>(v), 0.0f, 0.0f};
    }

    sfloat_rg32(extent2 const &rhs) noexcept : sfloat_rg32{static_cast<f32x4>(rhs)} {}
    sfloat_rg32(scale2 const &rhs) noexcept : sfloat_rg32{static_cast<f32x4>(rhs)} {}
    sfloat_rg32(vector2 const &rhs) noexcept : sfloat_rg32{static_cast<f32x4>(rhs)} {}
    sfloat_rg32(point2 const &rhs) noexcept : sfloat_rg32{static_cast<f32x4>(rhs)} {}

    [[nodiscard]] friend bool operator==(sfloat_rg32 const &lhs, sfloat_rg32 const &rhs) noexcept = default;
};

} // namespace hi::inline v1
