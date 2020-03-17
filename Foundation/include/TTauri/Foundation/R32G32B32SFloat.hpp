// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace TTauri {

class R32G32B32SFloat {
    // Red, Green, Blue, Alpha in binary32 (native endian).
    std::array<float,3> v;

public:
    force_inline R32G32B32SFloat() = default;
    force_inline R32G32B32SFloat(R32G32B32SFloat const &rhs) noexcept = default;
    force_inline R32G32B32SFloat(R32G32B32SFloat &&rhs) noexcept = default;
    force_inline R32G32B32SFloat &operator=(R32G32B32SFloat const &rhs) noexcept = default;
    force_inline R32G32B32SFloat &operator=(R32G32B32SFloat &&rhs) noexcept = default;

    force_inline R32G32B32SFloat(vec const &rhs) noexcept {
        alignas(16) std::array<float,4> tmp;
        _mm_storeu_ps(tmp.data(), rhs);
        std::memcpy(v.data(), tmp.data(), sizeof(v));
    }

    force_inline R32G32B32SFloat &operator=(vec const &rhs) noexcept {
        alignas(16) std::array<float,4> tmp;
        _mm_storeu_ps(tmp.data(), rhs);
        std::memcpy(v.data(), tmp.data(), sizeof(v));
        return *this;
    }

    force_inline operator vec () const noexcept {
        alignas(16) std::array<float,4> tmp;
        std::memcpy(tmp.data(), v.data(), sizeof(v));
        tmp[3] = 0.0;
        return _mm_loadu_ps(tmp.data());
    }

    [[nodiscard]] force_inline friend bool operator==(R32G32B32SFloat const &lhs, R32G32B32SFloat const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] force_inline friend bool operator!=(R32G32B32SFloat const &lhs, R32G32B32SFloat const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
