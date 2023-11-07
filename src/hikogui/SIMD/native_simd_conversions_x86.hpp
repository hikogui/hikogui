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
#include "../macros.hpp"

#ifdef HI_HAS_SSE
#include <xmmintrin.h>
#endif
#ifdef HI_HAS_SSE2
#include <emmintrin.h>
#endif
#ifdef HI_HAS_SSE3
#include <pmmintrin.h>
#endif
#ifdef HI_HAS_SSSE3
#include <tmmintrin.h>
#endif
#ifdef HI_HAS_SSE4_1
#include <smmintrin.h>
#endif
#ifdef HI_HAS_SSE4_2
#include <nmmintrin.h>
#endif
#ifdef HI_HAS_AVX
#include <immintrin.h>
#endif

hi_export_module(hikogui.SIMD : native_simd_conversions_x86);

hi_export namespace hi { inline namespace v1 {

#ifdef HI_HAS_SSE2
[[nodiscard]] hi_inline native_simd<float, 4>::native_simd(native_simd<int32_t, 4> const& a) noexcept : v(_mm_cvtepi32_ps(a.v)) {}
[[nodiscard]] hi_inline native_simd<int32_t, 4>::native_simd(native_simd<float, 4> const& a) noexcept : v(_mm_cvtps_epi32(a.v)) {}
[[nodiscard]] hi_inline native_simd<int32_t, 4>::native_simd(native_simd<uint32_t, 4> const& a) noexcept : v(a.v) {}
[[nodiscard]] hi_inline native_simd<uint32_t, 4>::native_simd(native_simd<int32_t, 4> const& a) noexcept : v(a.v) {}
#endif
#ifdef HI_HAS_AVX
[[nodiscard]] hi_inline native_simd<float, 4>::native_simd(native_simd<double, 4> const& a) noexcept : v(_mm256_cvtpd_ps(a.v)) {}
[[nodiscard]] hi_inline native_simd<double, 4>::native_simd(native_simd<float, 4> const& a) noexcept : v(_mm256_cvtps_pd(a.v)) {}
[[nodiscard]] hi_inline native_simd<double, 4>::native_simd(native_simd<int32_t, 4> const& a) noexcept : v(_mm256_cvtepi32_pd(a.v))
{
}
[[nodiscard]] hi_inline native_simd<int32_t, 4>::native_simd(native_simd<double, 4> const& a) noexcept : v(_mm256_cvtpd_epi32(a.v))
{
}
#endif
#ifdef HI_HAS_AVX2
[[nodiscard]] hi_inline native_simd<int64_t, 4>::native_simd(native_simd<int32_t, 4> const& a) noexcept :
    v(_mm256_cvtepi32_epi64(a.v))
{
}
[[nodiscard]] hi_inline native_simd<int64_t, 4>::native_simd(native_simd<uint32_t, 4> const& a) noexcept :
    v(_mm256_cvtepu32_epi64(a.v))
{
}
#endif


//[[nodiscard]] hi_inline native_f32x4 permute(native_f32x4 a, native_i32x4 const &source_elements) noexcept
//{
//    return native_f32x4{_mm_shufflevar_ps(a.v, source_elements.v)};
//}

}} // namespace hi::v1
