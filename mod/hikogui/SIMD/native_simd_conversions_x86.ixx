// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_SIMD : native_simd_conversions_x86;
import : native_i32x4_sse2;
import : native_f32x4_sse;
import : native_f64x4_avx;
import : native_u32x4_sse2;
import : native_i64x4_avx2;
import : native_simd_utility;

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


//[[nodiscard]] inline native_f32x4 permute(native_f32x4 a, native_i32x4 const &source_elements) noexcept
//{
//    return native_f32x4{_mm_shufflevar_ps(a.v, source_elements.v)};
//}

}} // namespace hi::v1
