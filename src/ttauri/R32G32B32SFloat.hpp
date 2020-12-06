// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "vec.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class R32G32B32SFloat {
    // Red, Green, Blue, Alpha in binary32 (native endian).
    std::array<float,3> v;

public:
    R32G32B32SFloat() = default;
    R32G32B32SFloat(R32G32B32SFloat const &rhs) noexcept = default;
    R32G32B32SFloat(R32G32B32SFloat &&rhs) noexcept = default;
    R32G32B32SFloat &operator=(R32G32B32SFloat const &rhs) noexcept = default;
    R32G32B32SFloat &operator=(R32G32B32SFloat &&rhs) noexcept = default;

    R32G32B32SFloat(f32x4 const &rhs) noexcept {
        alignas(16) std::array<float,4> tmp;
        _mm_storeu_ps(tmp.data(), rhs);
        std::memcpy(v.data(), tmp.data(), sizeof(v));
    }

    R32G32B32SFloat &operator=(f32x4 const &rhs) noexcept {
        alignas(16) std::array<float,4> tmp;
        _mm_storeu_ps(tmp.data(), rhs);
        std::memcpy(v.data(), tmp.data(), sizeof(v));
        return *this;
    }

    operator f32x4 () const noexcept {
        alignas(16) std::array<float,4> tmp;
        std::memcpy(tmp.data(), v.data(), sizeof(v));
        tmp[3] = 0.0;
        return _mm_loadu_ps(tmp.data());
    }

    [[nodiscard]] friend bool operator==(R32G32B32SFloat const &lhs, R32G32B32SFloat const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] friend bool operator!=(R32G32B32SFloat const &lhs, R32G32B32SFloat const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
