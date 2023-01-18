// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined(HI_HAS_SSE4_1)
#include <smmintrin.h> // SSE4.1
#include <ammintrin.h> // SSE4A
#endif
#if defined(HI_HAS_SSSE3)
#include <tmmintrin.h> // SSSE3
#endif
#if defined(HI_HAS_SSE3)
#include <pmmintrin.h> // SSE3
#endif
#if defined(HI_HAS_SSE2)
#include <emmintrin.h> // SSE2
#endif
#if defined(HI_HAS_SSE)
#include <xmmintrin.h> // SSE
#endif
#include "../utility/module.hpp"

namespace hi::inline v1 {

inline __m128 _mm_cvtph_ps_sse2(__m128i value) noexcept
{
    hilet f32_to_f16_constants = _mm_set_epi32(0, f32_to_f16_adjustment, f32_to_f16_infinite, f32_to_f16_lowest_normal - 1);

    // Convert the 16 bit values to 32 bit with leading zeros.
    auto u = _mm_unpacklo_epi16(value, _mm_setzero_si128()); // SSE2

    // Extract the sign bit.
    hilet sign = _mm_slli_epi32(_mm_srli_epi32(u, 15), 31); // SSE2

    // Strip the sign bit and align the exponent/mantissa boundary to a float 32.
    u = _mm_srli_epi32(_mm_slli_epi32(u, 17), 4); // SSE2

    // Adjust the bias. f32_to_f16_adjustment
    hilet f32_to_f16_adjustment_ = _mm_shuffle_epi32(f32_to_f16_constants, 0b10'10'10'10); // SSE2
    u = _mm_add_epi32(u, f32_to_f16_adjustment_); // SSE2

    // Get a mask of '1' bits when the half-float would be normal or infinite.
    hilet f32_to_f16_lowest_normal_ = _mm_shuffle_epi32(f32_to_f16_constants, 0b00'00'00'00); // SSE2
    hilet is_normal = _mm_cmpgt_epi32(u, f32_to_f16_lowest_normal_); // SSE2

    // Add the sign bit back in.
    u = _mm_or_si128(u, sign); // SSE2

    // Keep the value if normal, if denormal make it zero.
    u = _mm_and_si128(u, is_normal); // SSE2

    return _mm_castsi128_ps(u); // SSE2
}

inline __m128i _mm_cvtps_ph_sse4_1(__m128 value) noexcept
{
    hilet f32_to_f16_constants = _mm_set_epi32(0, f32_to_f16_adjustment, f32_to_f16_infinite, f32_to_f16_lowest_normal - 1);

    // Interpret the floating point number as 32 bit-field.
    auto u = _mm_castps_si128(value); // SSE2

    // Get the sign of the floating point number as a bit mask of the upper 17 bits.
    hilet sign = _mm_slli_epi32(_mm_srai_epi32(u, 31), 15); // SSE2

    // Strip sign bit.
    u = _mm_srli_epi32(_mm_slli_epi32(u, 1), 1); // SSE2

    // Get a mask of '1' bits when the half-float would be normal or infinite.
    hilet f32_to_f16_lowest_normal_ = _mm_shuffle_epi32(f32_to_f16_constants, 0b00'00'00'00); // SSE2
    hilet is_normal = _mm_cmpgt_epi32(u, f32_to_f16_lowest_normal_);

    // Clamp the floating point number to where the half-float would be infinite.
    hilet f32_to_f16_infinite_ = _mm_shuffle_epi32(f32_to_f16_constants, 0b01'01'01'01); // SSE2
    u = _mm_min_epi32(u, f32_to_f16_infinite_); // SSE4.1

    // Convert the bias from float to half-float.
    hilet f32_to_f16_adjustment_ = _mm_shuffle_epi32(f32_to_f16_constants, 0b10'10'10'10); // SSE2
    u = _mm_sub_epi32(u, f32_to_f16_adjustment_);

    // Shift the float until it becomes a half-float. This truncates the mantissa.
    u = _mm_srli_epi32(u, 13);

    // Keep the value if normal, if denormal make it zero.
    u = _mm_and_si128(u, is_normal);

    // Add the sign bit back in, also set the upper 16 bits so that saturated pack
    // will work correctly when converting to int16.
    u = _mm_or_si128(u, sign);

    // Saturate and pack the 32 bit integers to 16 bit integers.
    return _mm_packs_epi32(u, u);
}

} // namespace hi::inline v1
