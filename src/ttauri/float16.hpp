// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <emmintrin.h>
#include <immintrin.h>
#include <type_traits>

namespace tt {

class float16 {
    uint16_t v;

public:
    float16() noexcept : v() {}

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    float16(T const &rhs) noexcept {
        ttlet tmp1 = narrow_cast<float>(rhs);
        ttlet tmp2 = _mm_set_ss(tmp1);
        ttlet tmp3 = _mm_cvtps_ph(tmp2, _MM_FROUND_CUR_DIRECTION);
        _mm_storeu_si16(&v, tmp3);
    }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    float16 &operator=(T const &rhs) noexcept {
        ttlet tmp1 = narrow_cast<float>(rhs);
        ttlet tmp2 = _mm_set_ss(tmp1);
        ttlet tmp3 = _mm_cvtps_ph(tmp2, _MM_FROUND_CUR_DIRECTION);
        _mm_storeu_si16(&v, tmp3);
        return *this;
    }

    operator float () const noexcept {
        ttlet tmp1 = _mm_loadu_si16(&v);
        ttlet tmp2 = _mm_cvtph_ps(tmp1);
        return _mm_cvtss_f32(tmp2);
    }

    float16(uint16_t const &rhs, bool) noexcept : v(rhs) {}

    [[nodiscard]] constexpr uint16_t get() const noexcept {
        return v;
    }

    constexpr float16 &set(uint16_t rhs) noexcept {
        v = rhs;
        return *this;
    }

    [[nodiscard]] size_t hash() const noexcept
    {
        return std::hash<uint16_t>{}(v);
    }

    [[nodiscard]] friend bool operator==(float16 const &lhs, float16 const &rhs) noexcept {
        return lhs.v == rhs.v;
    }

    [[nodiscard]] friend bool operator!=(float16 const &lhs, float16 const &rhs) noexcept {
        return lhs.v != rhs.v;
    }
};

}

namespace std {

template<>
struct std::hash<tt::float16> {
    size_t operator()(tt::float16 const &rhs) noexcept
    {
        return rhs.hash();
    }
};

}
