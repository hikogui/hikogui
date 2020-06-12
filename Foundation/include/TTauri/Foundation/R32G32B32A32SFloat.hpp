// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class R32G32B32A32SFloat {
    // Red, Green, Blue, Alpha in binary32 (native endian).
    std::array<float,4> v;

public:
    force_inline R32G32B32A32SFloat() = default;
    force_inline R32G32B32A32SFloat(R32G32B32A32SFloat const &rhs) noexcept = default;
    force_inline R32G32B32A32SFloat(R32G32B32A32SFloat &&rhs) noexcept = default;
    force_inline R32G32B32A32SFloat &operator=(R32G32B32A32SFloat const &rhs) noexcept = default;
    force_inline R32G32B32A32SFloat &operator=(R32G32B32A32SFloat &&rhs) noexcept = default;

    force_inline R32G32B32A32SFloat(vec const &rhs) noexcept {
        _mm_storeu_ps(v.data(), rhs);
    }

    force_inline R32G32B32A32SFloat &operator=(vec const &rhs) noexcept {
        _mm_storeu_ps(v.data(), rhs);
        return *this;
    }

    force_inline operator vec () const noexcept {
        return _mm_loadu_ps(v.data());
    }

    force_inline R32G32B32A32SFloat(aarect const &rhs) noexcept {
        _mm_storeu_ps(v.data(), rhs.v);
    }

    force_inline R32G32B32A32SFloat &operator=(aarect const &rhs) noexcept {
        _mm_storeu_ps(v.data(), rhs.v);
        return *this;
    }

    force_inline operator aarect () const noexcept {
        return aarect::p0p3(_mm_loadu_ps(v.data()));
    }

    [[nodiscard]] force_inline friend bool operator==(R32G32B32A32SFloat const &lhs, R32G32B32A32SFloat const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] force_inline friend bool operator!=(R32G32B32A32SFloat const &lhs, R32G32B32A32SFloat const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
