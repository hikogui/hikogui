// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/vec.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class R32G32SFloat {
    alignas(sizeof(float) * 2)
    // Red, Green, Blue, Alpha in binary32 (native endian).
    std::array<float,2> v;

public:
    tt_force_inline R32G32SFloat() = default;
    tt_force_inline R32G32SFloat(R32G32SFloat const &rhs) noexcept = default;
    tt_force_inline R32G32SFloat(R32G32SFloat &&rhs) noexcept = default;
    tt_force_inline R32G32SFloat &operator=(R32G32SFloat const &rhs) noexcept = default;
    tt_force_inline R32G32SFloat &operator=(R32G32SFloat &&rhs) noexcept = default;

    tt_force_inline R32G32SFloat(vec const &rhs) noexcept :
        v(static_cast<decltype(v)>(rhs)) {}

    tt_force_inline R32G32SFloat &operator=(vec const &rhs) noexcept {
        v = static_cast<decltype(v)>(rhs);
        return *this;
    }

    tt_force_inline operator vec () const noexcept {
        return vec{v};
    }

    [[nodiscard]] tt_force_inline friend bool operator==(R32G32SFloat const &lhs, R32G32SFloat const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] tt_force_inline friend bool operator!=(R32G32SFloat const &lhs, R32G32SFloat const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
