// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "vec.hpp"
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

    R32G32B32A32SFloat(f32x4 const &rhs) noexcept {
        _mm_storeu_ps(v.data(), rhs);
    }

    R32G32B32A32SFloat &operator=(f32x4 const &rhs) noexcept {
        _mm_storeu_ps(v.data(), rhs);
        return *this;
    }

    operator f32x4 () const noexcept {
        return _mm_loadu_ps(v.data());
    }

    R32G32B32A32SFloat(aarect const &rhs) noexcept {
        _mm_storeu_ps(v.data(), rhs.v);
    }

    R32G32B32A32SFloat &operator=(aarect const &rhs) noexcept {
        _mm_storeu_ps(v.data(), rhs.v);
        return *this;
    }

    operator aarect () const noexcept {
        return aarect::p0p3(_mm_loadu_ps(v.data()));
    }

    [[nodiscard]] friend bool operator==(R32G32B32A32SFloat const &lhs, R32G32B32A32SFloat const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] friend bool operator!=(R32G32B32A32SFloat const &lhs, R32G32B32A32SFloat const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
