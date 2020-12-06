// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "vec.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class R32G32SFloat {
    alignas(sizeof(float) * 2)
    // Red, Green, Blue, Alpha in binary32 (native endian).
    std::array<float,2> v;

public:
    R32G32SFloat() = default;
    R32G32SFloat(R32G32SFloat const &rhs) noexcept = default;
    R32G32SFloat(R32G32SFloat &&rhs) noexcept = default;
    R32G32SFloat &operator=(R32G32SFloat const &rhs) noexcept = default;
    R32G32SFloat &operator=(R32G32SFloat &&rhs) noexcept = default;

    R32G32SFloat(f32x4 const &rhs) noexcept :
        v(static_cast<decltype(v)>(rhs)) {}

    R32G32SFloat &operator=(f32x4 const &rhs) noexcept {
        v = static_cast<decltype(v)>(rhs);
        return *this;
    }

    operator f32x4 () const noexcept {
        return f32x4{v};
    }

    [[nodiscard]] friend bool operator==(R32G32SFloat const &lhs, R32G32SFloat const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] friend bool operator!=(R32G32SFloat const &lhs, R32G32SFloat const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
