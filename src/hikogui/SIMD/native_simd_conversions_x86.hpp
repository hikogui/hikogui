// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "native_f32x4_sse.hpp"
#include "native_f64x4_avx.hpp"
#include "native_i32x4_sse2.hpp"
#include "native_i64x4_avx2.hpp"
#include "native_u32x4_sse2.hpp"
#include "native_simd_utility.hpp"

namespace hi { inline namespace v1 {

#ifdef HI_HAS_SSE2
[[nodiscard]] inline native_simd<float, 4>::native_simd(native_simd<int32_t, 4> const& a) noexcept : v(_mm_cvtepi32_ps(a.v)) {}
[[nodiscard]] inline native_simd<int32_t, 4>::native_simd(native_simd<float, 4> const& a) noexcept : v(_mm_cvtps_epi32(a.v)) {}
[[nodiscard]] inline native_simd<int32_t, 4>::native_simd(native_simd<uint32_t, 4> const& a) noexcept : v(a.v) {}
[[nodiscard]] inline native_simd<uint32_t, 4>::native_simd(native_simd<int32_t, 4> const& a) noexcept : v(a.v) {}
#endif
#ifdef HI_HAS_AVX
[[nodiscard]] inline native_simd<float, 4>::native_simd(native_simd<double, 4> const& a) noexcept : v(_mm256_cvtpd_ps(a.v)) {}
[[nodiscard]] inline native_simd<double, 4>::native_simd(native_simd<float, 4> const& a) noexcept : v(_mm256_cvtps_pd(a.v)) {}
[[nodiscard]] inline native_simd<double, 4>::native_simd(native_simd<int32_t, 4> const& a) noexcept : v(_mm256_cvtepi32_pd(a.v))
{
}
[[nodiscard]] inline native_simd<int32_t, 4>::native_simd(native_simd<double, 4> const& a) noexcept : v(_mm256_cvtpd_epi32(a.v))
{
}
#endif
#ifdef HI_HAS_AVX2
[[nodiscard]] inline native_simd<int64_t, 4>::native_simd(native_simd<int32_t, 4> const& a) noexcept :
    v(_mm256_cvtepi32_epi64(a.v))
{
}
[[nodiscard]] inline native_simd<int64_t, 4>::native_simd(native_simd<uint32_t, 4> const& a) noexcept :
    v(_mm256_cvtepu32_epi64(a.v))
{
}
#endif

#ifdef HI_HAS_SSE2
native_simd<float16,8>::native_simd(native_simd<float,8> const &a) noexcept
{
#ifdef HI_HAS_F16C
    v = _mm256_cvtps_ph(a.v, _MM_FROUND_CUR_DIRECTION);
#else
#endif
}

native_simd<float16,8>::native_simd(native_simd<float,4> const &a, simd<float,4> const &b) noexcept
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
#endif

//[[nodiscard]] inline native_f32x4 permute(native_f32x4 a, native_i32x4 const &source_elements) noexcept
//{
//    return native_f32x4{_mm_shufflevar_ps(a.v, source_elements.v)};
//}

}} // namespace hi::v1
