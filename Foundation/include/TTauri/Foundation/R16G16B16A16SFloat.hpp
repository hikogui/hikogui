// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/float16.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace TTauri {

class R16G16B16A16SFloat {
    // Red, Green, Blue, Alpha in binary16 (native endian).
    std::array<float16,4> v;

public:
    force_inline R16G16B16A16SFloat() = default;
    force_inline R16G16B16A16SFloat(R16G16B16A16SFloat const &rhs) noexcept = default;
    force_inline R16G16B16A16SFloat(R16G16B16A16SFloat &&rhs) noexcept = default;
    force_inline R16G16B16A16SFloat &operator=(R16G16B16A16SFloat const &rhs) noexcept = default;
    force_inline R16G16B16A16SFloat &operator=(R16G16B16A16SFloat &&rhs) noexcept = default;

    force_inline R16G16B16A16SFloat(vec const &rhs) noexcept {
        let rhs_fp16 = _mm_cvtps_ph(rhs, _MM_FROUND_CUR_DIRECTION);
        _mm_storeu_si64(v.data(), rhs_fp16);
    }

    force_inline R16G16B16A16SFloat &operator=(vec const &rhs) noexcept {
        let rhs_fp16 = _mm_cvtps_ph(rhs, _MM_FROUND_CUR_DIRECTION);
        _mm_storeu_si64(v.data(), rhs_fp16);
        return *this;
    }

    force_inline operator vec () const noexcept {
        let rhs_fp16 = _mm_loadu_si64(v.data());
        return _mm_cvtph_ps(rhs_fp16);
    }

    std::array<float16,4> const &get() const noexcept {
        return v;
    }

    [[nodiscard]] force_inline friend bool operator==(R16G16B16A16SFloat const &lhs, R16G16B16A16SFloat const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] force_inline friend bool operator!=(R16G16B16A16SFloat const &lhs, R16G16B16A16SFloat const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}
