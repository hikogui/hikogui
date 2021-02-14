// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../numeric_array.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class sfloat_rg32 {
    alignas(sizeof(float) * 2)
    // Red, Green in binary32 (native endian).
    std::array<float,2> v;

public:
    sfloat_rg32() = default;
    sfloat_rg32(sfloat_rg32 const &rhs) noexcept = default;
    sfloat_rg32(sfloat_rg32 &&rhs) noexcept = default;
    sfloat_rg32 &operator=(sfloat_rg32 const &rhs) noexcept = default;
    sfloat_rg32 &operator=(sfloat_rg32 &&rhs) noexcept = default;

    sfloat_rg32(f32x4 const &rhs) noexcept :
        v(static_cast<decltype(v)>(rhs)) {}

    sfloat_rg32 &operator=(f32x4 const &rhs) noexcept {
        v = static_cast<decltype(v)>(rhs);
        return *this;
    }

    operator f32x4 () const noexcept {
        return f32x4{v};
    }

    [[nodiscard]] friend bool operator==(sfloat_rg32 const &lhs, sfloat_rg32 const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] friend bool operator!=(sfloat_rg32 const &lhs, sfloat_rg32 const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
