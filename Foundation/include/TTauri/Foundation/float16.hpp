// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <type_traits>

namespace TTauri {

class float16 {
    uint16_t v;

public:
    float16() noexcept : v() {}

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    float16(T const &rhs) noexcept {
        let tmp1 = numeric_cast<float>(rhs);
        let tmp2 = _mm_set_ss(tmp1);
        let tmp3 = _mm_cvtps_ph(tmp2, _MM_FROUND_CUR_DIRECTION);
        _mm_storeu_si16(&v, tmp3);
    }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    float16 &operator=(T const &rhs) noexcept {
        let tmp1 = numeric_cast<float>(rhs);
        let tmp2 = _mm_set_ss(tmp1);
        let tmp3 = _mm_cvtps_ph(tmp2, _MM_FROUND_CUR_DIRECTION);
        _mm_storeu_si16(&v, tmp3);
        return *this;
    }

    operator float () const noexcept {
        let tmp1 = _mm_loadu_si16(&v);
        let tmp2 = _mm_cvtph_ps(tmp1);
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
};

}
