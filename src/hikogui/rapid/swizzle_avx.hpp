// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility.hpp"
#include "../architecture.hpp"
#if defined(HI_HAS_AVX)

#include <emmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>

hi_warning_push();
// C26818: Switch statement does not cover all cases. Consider adding a 'default' label (es.79).
// False positive.
hi_warning_ignore_msvc(26818)

namespace hi::inline v1 {

template<ssize_t A, ssize_t B, ssize_t C, ssize_t D>
[[nodiscard]] constexpr static int _mm_swizzle_ps_permute_mask() noexcept
{
    static_assert(A >= -3 and A < 4);
    static_assert(B >= -3 and B < 4);
    static_assert(C >= -3 and C < 4);
    static_assert(D >= -3 and D < 4);

    int r = 0;
    switch (A) {
    case 0: r |= 0b00'00'00'00; break;
    case 1: r |= 0b00'00'00'01; break;
    case 2: r |= 0b00'00'00'10; break;
    case 3: r |= 0b00'00'00'11; break;
    case -1: r |= 0b00'00'00'00; break;
    case -2: r |= 0b00'00'00'00; break;
    }
    switch (B) {
    case 0: r |= 0b00'00'00'00; break;
    case 1: r |= 0b00'00'01'00; break;
    case 2: r |= 0b00'00'10'00; break;
    case 3: r |= 0b00'00'11'00; break;
    case -1: r |= 0b00'00'01'00; break;
    case -2: r |= 0b00'00'01'00; break;
    }
    switch (C) {
    case 0: r |= 0b00'00'00'00; break;
    case 1: r |= 0b00'01'00'00; break;
    case 2: r |= 0b00'10'00'00; break;
    case 3: r |= 0b00'11'00'00; break;
    case -1: r |= 0b00'10'00'00; break;
    case -2: r |= 0b00'10'00'00; break;
    }
    switch (D) {
    case 0: r |= 0b00'00'00'00; break;
    case 1: r |= 0b01'00'00'00; break;
    case 2: r |= 0b10'00'00'00; break;
    case 3: r |= 0b11'00'00'00; break;
    case -1: r |= 0b11'00'00'00; break;
    case -2: r |= 0b11'00'00'00; break;
    }
    return r;
}

template<ssize_t A, ssize_t B, ssize_t C, ssize_t D>
[[nodiscard]] constexpr static int _mm_swizzle_ps_not_one_mask() noexcept
{
    static_assert(A >= -3 && A < 4);
    static_assert(B >= -3 && B < 4);
    static_assert(C >= -3 && C < 4);
    static_assert(D >= -3 && D < 4);

    int r = 0;
    r |= (A == -2) ? 0 : 0b0001;
    r |= (B == -2) ? 0 : 0b0010;
    r |= (C == -2) ? 0 : 0b0100;
    r |= (D == -2) ? 0 : 0b1000;
    return r;
}

template<ssize_t A, ssize_t B, ssize_t C, ssize_t D>
[[nodiscard]] constexpr static int _mm_swizzle_ps_number_mask() noexcept
{
    static_assert(A >= -3 && A < 4);
    static_assert(B >= -3 && B < 4);
    static_assert(C >= -3 && C < 4);
    static_assert(D >= -3 && D < 4);

    int r = 0;
    r |= A < 0 ? 0b0001 : 0;
    r |= B < 0 ? 0b0010 : 0;
    r |= C < 0 ? 0b0100 : 0;
    r |= D < 0 ? 0b1000 : 0;
    return r;
}

template<ssize_t A, ssize_t B, ssize_t C, ssize_t D>
[[nodiscard]] __m128 _mm_swizzle_ps(__m128 const &value) noexcept
{
    static_assert(A >= -3 && A < 4);
    static_assert(B >= -3 && B < 4);
    static_assert(C >= -3 && C < 4);
    static_assert(D >= -3 && D < 4);

    constexpr int permute_mask = _mm_swizzle_ps_permute_mask<A, B, C, D>();
    constexpr int not_one_mask = _mm_swizzle_ps_not_one_mask<A, B, C, D>();
    constexpr int number_mask = _mm_swizzle_ps_number_mask<A, B, C, D>();

    __m128 swizzled;
    // Clang is able to optimize these intrinsics, MSVC is not.
    if constexpr (permute_mask != 0b11'10'01'00) {
        swizzled = _mm_permute_ps(value, permute_mask);
    } else {
        swizzled = value;
    }

    __m128 numbers = _mm_undefined_ps();
    if constexpr (not_one_mask == 0b0000) {
        numbers = _mm_set_ps1(1.0f);
    } else if constexpr (not_one_mask == 0b1111) {
        numbers = _mm_setzero_ps();
    } else if constexpr (not_one_mask == 0b1110) {
        numbers = _mm_set_ss(1.0f);
    } else {
        hilet _1111 = _mm_set_ps1(1.0f);
        numbers = _mm_insert_ps(_1111, _1111, not_one_mask);
    }

    __m128 result = _mm_undefined_ps();
    if constexpr (number_mask == 0b0000) {
        result = swizzled;
    } else if constexpr (number_mask == 0b1111) {
        result = numbers;
    } else if constexpr (((not_one_mask | ~number_mask) & 0b1111) == 0b1111) {
        result = _mm_insert_ps(swizzled, swizzled, number_mask);
    } else {
        result = _mm_blend_ps(swizzled, numbers, number_mask);
    }
    return result;
}

template<ssize_t A, ssize_t B, ssize_t C, ssize_t D>
[[nodiscard]] __m128i _mm_swizzle_epi32(__m128i const &value) noexcept
{
    return _mm_castps_si128(_mm_swizzle_ps<A, B, C, D>(_mm_castsi128_ps(value)));
}

template<ssize_t A = -1, ssize_t B = -1>
[[nodiscard]] __m128d _mm_swizzle_pd(__m128d const &value) noexcept
{
    constexpr auto A1 = A >= 0 ? A * 2 : A;
    constexpr auto A2 = A >= 0 ? A1 + 1 : A1;
    constexpr auto B1 = B >= 0 ? B * 2 : B;
    constexpr auto B2 = B >= 0 ? B1 + 1 : B1;

    return _mm_castps_pd(_mm_swizzle_ps<A1, A2, B1, B2>(_mm_castpd_ps(value)));
}

template<ssize_t A = -1, ssize_t B = -1>
[[nodiscard]] __m128i _mm_swizzle_epi64(__m128i const &value) noexcept
{
    return _mm_castpd_si128(_mm_swizzle_pd<A, B>(_mm_castsi128_pd(value)));
}

} // namespace hi::inline v1

hi_warning_pop();

#endif
