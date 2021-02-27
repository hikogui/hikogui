// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../numeric_array.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class sfloat_rgb32 {
    // Red, Green, Blue in binary32 (native endian).
    std::array<float,3> v;

public:
    sfloat_rgb32() = default;
    sfloat_rgb32(sfloat_rgb32 const &rhs) noexcept = default;
    sfloat_rgb32(sfloat_rgb32 &&rhs) noexcept = default;
    sfloat_rgb32 &operator=(sfloat_rgb32 const &rhs) noexcept = default;
    sfloat_rgb32 &operator=(sfloat_rgb32 &&rhs) noexcept = default;

    sfloat_rgb32(f32x4 const &rhs) noexcept : v(static_cast<std::array<float,3>>(rhs)) {}

    sfloat_rgb32 &operator=(f32x4 const &rhs) noexcept {
        v = static_cast<std::array<float, 3>>(rhs);
        return *this;
    }

    operator f32x4 () const noexcept {
        return f32x4{v};
    }

    sfloat_rgb32(point3 const &rhs) noexcept : v(static_cast<std::array<float, 3>>(static_cast<f32x4>(rhs))) {}

    sfloat_rgb32 &operator=(point3 const &rhs) noexcept
    {
        v = static_cast<std::array<float, 3>>(static_cast<f32x4>(rhs));
        return *this;
    }

    operator point3() const noexcept
    {
        return point3{f32x4{v}};
    }

    [[nodiscard]] friend bool operator==(sfloat_rgb32 const &lhs, sfloat_rgb32 const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] friend bool operator!=(sfloat_rgb32 const &lhs, sfloat_rgb32 const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
