// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "numeric_array.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class R32G32B32SFloat {
    // Red, Green, Blue in binary32 (native endian).
    std::array<float,3> v;

public:
    R32G32B32SFloat() = default;
    R32G32B32SFloat(R32G32B32SFloat const &rhs) noexcept = default;
    R32G32B32SFloat(R32G32B32SFloat &&rhs) noexcept = default;
    R32G32B32SFloat &operator=(R32G32B32SFloat const &rhs) noexcept = default;
    R32G32B32SFloat &operator=(R32G32B32SFloat &&rhs) noexcept = default;

    R32G32B32SFloat(f32x4 const &rhs) noexcept : v(static_cast<std::array<float,3>>(rhs)) {}

    R32G32B32SFloat &operator=(f32x4 const &rhs) noexcept {
        v = static_cast<std::array<float, 3>>(rhs);
        return *this;
    }

    operator f32x4 () const noexcept {
        return f32x4{v};
    }

    [[nodiscard]] friend bool operator==(R32G32B32SFloat const &lhs, R32G32B32SFloat const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] friend bool operator!=(R32G32B32SFloat const &lhs, R32G32B32SFloat const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
