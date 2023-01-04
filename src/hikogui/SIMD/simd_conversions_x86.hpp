// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd_f32x4_sse.hpp"
#include "simd_f64x4_avx.hpp"
#include "simd_i32x4_sse2.hpp"
#include "simd_i64x4_avx2.hpp"
#include "simd_u32x4_sse2.hpp"
#include "simd_utility.hpp"

namespace hi {
inline namespace v1 {

#ifdef HI_HAS_SSE2
[[nodiscard]] inline simd_f32x4::simd_f32x4(simd_i32x4 const& a) noexcept : v(_mm_cvtepi32_ps(a.v)) {}
[[nodiscard]] inline simd_i32x4::simd_i32x4(simd_f32x4 const& a) noexcept : v(_mm_cvtps_epi32(a.v)) {}
[[nodiscard]] inline simd_i32x4::simd_i32x4(simd_u32x4 const& a) noexcept : v(a.v) {}
[[nodiscard]] inline simd_u32x4::simd_u32x4(simd_i32x4 const& a) noexcept : v(a.v) {}
#endif
#ifdef HI_HAS_AVX
[[nodiscard]] inline simd_f32x4::simd_f32x4(simd_f64x4 const& a) noexcept : v(_mm256_cvtpd_ps(a.v)) {}
[[nodiscard]] inline simd_f64x4::simd_f64x4(simd_f32x4 const& a) noexcept : v(_mm256_cvtps_pd(a.v)) {}
[[nodiscard]] inline simd_f64x4::simd_f64x4(simd_i32x4 const& a) noexcept : v(_mm256_cvtepi32_pd(a.v)) {}
[[nodiscard]] inline simd_i32x4::simd_i32x4(simd_f64x4 const& a) noexcept : v(_mm256_cvtpd_epi32(a.v)) {}
#endif
#ifdef HI_HAS_AVX2
[[nodiscard]] inline simd_i64x4::simd_i64x4(simd_i32x4 const& a) noexcept : v(_mm256_cvtepi32_epi64(a.v)) {}
[[nodiscard]] inline simd_i64x4::simd_i64x4(simd_u32x4 const& a) noexcept : v(_mm256_cvtepu32_epi64(a.v)) {}
#endif

}}
