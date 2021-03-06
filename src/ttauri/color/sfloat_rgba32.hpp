// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/numeric_array.hpp"
#include "../geometry/corner_shapes.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "color.hpp"
#include <algorithm>

namespace tt {

class sfloat_rgba32 {
    // Red, Green, Blue, Alpha in binary32 (native endian).
    std::array<float, 4> v;

public:
    sfloat_rgba32() = default;
    sfloat_rgba32(sfloat_rgba32 const &rhs) noexcept = default;
    sfloat_rgba32(sfloat_rgba32 &&rhs) noexcept = default;
    sfloat_rgba32 &operator=(sfloat_rgba32 const &rhs) noexcept = default;
    sfloat_rgba32 &operator=(sfloat_rgba32 &&rhs) noexcept = default;

    sfloat_rgba32(f32x4 const &rhs) noexcept : v(static_cast<std::array<float, 4>>(rhs)) {}

    sfloat_rgba32 &operator=(f32x4 const &rhs) noexcept
    {
        v = static_cast<std::array<float, 4>>(rhs);
        return *this;
    }

    operator f32x4() const noexcept
    {
        return f32x4{v};
    }

    sfloat_rgba32(aarectangle const &rhs) noexcept : sfloat_rgba32(rhs.v) {}

    sfloat_rgba32(corner_shapes const &rhs) noexcept : sfloat_rgba32(static_cast<f32x4>(rhs)) {}

    sfloat_rgba32 &operator=(aarectangle const &rhs) noexcept
    {
        *this = rhs.v;
        return *this;
    }

    operator aarectangle() const noexcept
    {
        return aarectangle{f32x4{v}};
    }

    [[nodiscard]] friend bool operator==(sfloat_rgba32 const &lhs, sfloat_rgba32 const &rhs) noexcept
    {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] friend bool operator!=(sfloat_rgba32 const &lhs, sfloat_rgba32 const &rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

} // namespace tt
