// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "numeric_array.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class R32G32B32A32SFloat {
    // Red, Green, Blue, Alpha in binary32 (native endian).
    std::array<float,4> v;

public:
    R32G32B32A32SFloat() = default;
    R32G32B32A32SFloat(R32G32B32A32SFloat const &rhs) noexcept = default;
    R32G32B32A32SFloat(R32G32B32A32SFloat &&rhs) noexcept = default;
    R32G32B32A32SFloat &operator=(R32G32B32A32SFloat const &rhs) noexcept = default;
    R32G32B32A32SFloat &operator=(R32G32B32A32SFloat &&rhs) noexcept = default;

    R32G32B32A32SFloat(f32x4 const &rhs) noexcept : v(static_cast<std::array<float,4>>(rhs)) {
    }

    R32G32B32A32SFloat &operator=(f32x4 const &rhs) noexcept {
        v = static_cast<std::array<float,4>>(rhs);
        return *this;
    }

    operator f32x4 () const noexcept {
        return f32x4{v};
    }

    R32G32B32A32SFloat(aarect const &rhs) noexcept : R32G32B32A32SFloat(rhs.v) {}

    R32G32B32A32SFloat &operator=(aarect const &rhs) noexcept {
        *this = rhs.v;
        return *this;
    }

    operator aarect () const noexcept {
        return aarect::p0p3(f32x4(v));
    }

    [[nodiscard]] friend bool operator==(R32G32B32A32SFloat const &lhs, R32G32B32A32SFloat const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] friend bool operator!=(R32G32B32A32SFloat const &lhs, R32G32B32A32SFloat const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
