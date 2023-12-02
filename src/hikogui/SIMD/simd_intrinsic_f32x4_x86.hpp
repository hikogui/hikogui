// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd_intrinsic.hpp"
#include "../macros.hpp"
#include <cstddef>
#include <array>
#include <limits>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>

hi_export_module(hikogui.simd.f32x4);

hi_export namespace hi { inline namespace v1 {

#if defined(HI_HAS_SSE)
template<>
struct simd_intrinsic<float, 4> {
    using reg = __m128;
    using index_reg = __m128;
    using mask_reg = __m128;

    // clang-format off
    [[nodiscard]] hi_force_inline reg undefined() const noexcept { return _mm_undefined_ps(); }
    [[nodiscard]] hi_force_inline reg set_zero() const noexcept { return _mm_setzero_ps(); }

#if defined(HI_HAS_SSE2)
    [[nodiscard]] hi_force_inline mask_reg set_all_ones() const noexcept {
        return _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
    }
#else
    [[nodiscard]] hi_force_inline mask_reg set_all_ones() const noexcept { return _mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps()); }
#endif

#if defined(HI_HAS_SSE2)
    [[nodiscard]] hi_force_inline reg set_one() const noexcept {
        return _mm_castsi128_ps(_mm_srli_epi32(_mm_slli_epi32(_mm_castps_si128(set_all_ones()), 25), 2));
    }
#else
    [[nodiscard]] hi_force_inline reg set_one() const noexcept { return _mm_set1_ps(1.0f); }
#endif

    [[nodiscard]] hi_force_inline reg set(float a, float b, float c, float d) const noexcept { return _mm_set_ps(d, c, b, a); }

    [[nodiscard]] hi_force_inline reg set(float a) const noexcept
    {
        return _mm_set_ps(0.0f, 0.0f, 0.0f, a);
    }

    [[nodiscard]] hi_force_inline reg broadcast(float a) const noexcept { return _mm_set1_ps(a); }
    [[nodiscard]] hi_force_inline reg broadcast(reg a) const noexcept { return _mm_shuffle_ps(a, a, 0); }

    template<size_t I>
    [[nodiscard]] hi_force_inline float get(reg a) const noexcept
    {
        if constexpr (I == 0) {
            return _mm_cvtss_f32(a);
        } else {
#if defined(HI_HAS_SSE4_1)
            return _mm_extract_ps(a, I);
#else
            return _mm_cvtss_f32(_mm_shuffle_ps(a, a, 0b11'10'01'00 | I));
#endif
        }
    }

    [[nodiscard]] hi_force_inline reg load(float const *a) const noexcept { return _mm_loadu_ps(a); }

    [[nodiscard]] hi_force_inline std::array<float, 4> store(reg a) const noexcept {
        auto r = std::array<float, 4>{};
        _mm_storeu_ps(r.data(), a);
        return r;
    }

    [[nodiscard]] hi_force_inline reg neg(reg a) const noexcept { return _mm_sub_ps(_mm_setzero_ps(), a); }
    [[nodiscard]] hi_force_inline reg inv(reg a) const noexcept { return _mm_xor_ps(set_all_ones(), a); }

    [[nodiscard]] hi_force_inline reg rcp(reg a) const noexcept { return _mm_rcp_ps(a); }
    [[nodiscard]] hi_force_inline reg sqrt(reg a) const noexcept { return _mm_sqrt_ps(a); }
    [[nodiscard]] hi_force_inline reg rsqrt(reg a) const noexcept { return _mm_rsqrt_ps(a); }
#if defined(HI_HAS_SSE4_1)
    [[nodiscard]] hi_force_inline reg floor(reg a) const noexcept { return _mm_floor_ps(a); }
    [[nodiscard]] hi_force_inline reg ceil(reg a) const noexcept { return _mm_ceil_ps(a); }
#endif

#if defined(HI_HAS_SSE4_1)
    [[nodiscard]] hi_force_inline reg round(reg a) const noexcept { return _mm_round_ps(a, _MM_FROUND_CUR_DIRECTION); }
#elif defined(HI_HAS_SSE2)
    [[nodiscard]] hi_force_inline reg round(reg a) const noexcept
    {
        auto const rounded = _mm_cvtepi32_ps(_mm_cvtps_epi32(a));
        auto const check_max = _mm_cmple_ps(a, _mm_set1_ps(static_cast<float>(std::numeric_limits<int32_t>::max())));
        auto const check_min = _mm_cmpge_ps(a, _mm_set1_ps(static_cast<float>(std::numeric_limits<int32_t>::min())));
        auto const check_bounds = _mm_and_ps(check_max, check_min);

        auto const good_rounded = _mm_and_ps(check_bounds, rounded);
        auto const good_a = _mm_andnot_ps(check_bounds, a);
        return _mm_or_ps(good_rounded, good_a);
    }
#endif

    [[nodiscard]] hi_force_inline reg add(reg a, reg b) const noexcept { return _mm_add_ps(a, b); }
    [[nodiscard]] hi_force_inline reg sub(reg a, reg b) const noexcept { return _mm_sub_ps(a, b); }
    [[nodiscard]] hi_force_inline reg mul(reg a, reg b) const noexcept { return _mm_mul_ps(a, b); }
    [[nodiscard]] hi_force_inline reg div(reg a, reg b) const noexcept { return _mm_div_ps(a, b); }
    [[nodiscard]] hi_force_inline reg max(reg a, reg b) const noexcept { return _mm_max_ps(a, b); }
    [[nodiscard]] hi_force_inline reg min(reg a, reg b) const noexcept { return _mm_min_ps(a, b); }
    [[nodiscard]] hi_force_inline mask_reg eq(reg a, reg b) const noexcept { return _mm_cmpeq_ps(a, b); }
    [[nodiscard]] hi_force_inline mask_reg ne(reg a, reg b) const noexcept { return _mm_cmpneq_ps(a, b); }
    [[nodiscard]] hi_force_inline mask_reg lt(reg a, reg b) const noexcept { return _mm_cmplt_ps(a, b); }
    [[nodiscard]] hi_force_inline mask_reg gt(reg a, reg b) const noexcept { return _mm_cmpgt_ps(a, b); }
    [[nodiscard]] hi_force_inline mask_reg le(reg a, reg b) const noexcept { return _mm_cmple_ps(a, b); }
    [[nodiscard]] hi_force_inline mask_reg ge(reg a, reg b) const noexcept { return _mm_cmpge_ps(a, b); }
    [[nodiscard]] hi_force_inline mask_reg _or(mask_reg a, mask_reg b) const noexcept { return _mm_or_ps(a, b); }
    [[nodiscard]] hi_force_inline mask_reg _and(mask_reg a, mask_reg b) const noexcept { return _mm_and_ps(a, b); }
    [[nodiscard]] hi_force_inline mask_reg _xor(mask_reg a, mask_reg b) const noexcept { return _mm_xor_ps(a, b); }
    [[nodiscard]] hi_force_inline mask_reg andnot(mask_reg a, mask_reg b) const noexcept { return _mm_andnot_ps(a, b); }

    [[nodiscard]] hi_force_inline size_t mask_to_int(mask_reg a) const noexcept { return _mm_movemask_ps(a); }

#if defined(HI_HAS_SSE2)
    [[nodiscard]] hi_force_inline mask_reg int_to_mask(size_t mask) const noexcept
    {
        auto a = set(static_cast<int32_t>(mask) << 31);
        auto b = set(static_cast<int32_t>(mask) << 30);
        auto c = set(static_cast<int32_t>(mask) << 29);
        auto d = set(static_cast<int32_t>(mask) << 28);
        auto lo = _mm_castsi128_ps(_mm_unpacklo_epi32(a, b));
        auto hi = _mm_castsi128_ps(_mm_unpacklo_epi32(c, d));

        auto tmp = _mm_castps_si128(_mm_shuffle_ps(lo, hi, 0b01'00'01'00));
        return _mm_castsi128_ps(_mm_srai_epi32(tmp, 31));
    }
#else
    [[nodiscard]] hi_force_inline mask_reg int_to_mask(size_t mask) const noexcept
    {
        auto mask_ = static_cast<int64_t>(mask) << 31;

        auto r = std::array<int32_t, 4>{};
        std::get<0>(r) = static_cast<int32_t>(mask_) >> 31;
        mask_ >>= 1;
        std::get<1>(r) = static_cast<int32_t>(mask_) >> 31;
        mask_ >>= 1;
        std::get<2>(r) = static_cast<int32_t>(mask_) >> 31;
        mask_ >>= 1;
        std::get<3>(r) = static_cast<int32_t>(mask_) >> 31;
        return _mm_loadu_ps(reinterpret_cast<float const *>(r.data()));
    }
#endif

#if defined(HI_HAS_SSE4_1)
    [[nodiscard]] hi_force_inline bool test(reg a, reg b) const noexcept
    {
        return static_cast<bool>(_mm_testz_si128(_mm_castps_si128(a), _mm_castps_si128(b)));
    }
#elif defined(HI_HAS_SSE2)
    [[nodiscard]] hi_force_inline bool test(reg a, reg b) const noexcept
    {
        return _mm_movemask_epi8(_mm_cmpeq_epi32(_mm_castps_si128(_mm_and_ps(a, b)), _mm_setzero_si128())) == 0xffff;
    }
#else
    [[nodiscard]] hi_force_inline bool test(reg a, reg b) const noexcept
    {
        auto tmp = std::array<float, 4>{};
        _mm_store_ps(tmp.data(), _mm_and_ps(a, b));

        return (
            std::bit_cast<uint32_t>(std::get<0>(tmp)) |
            std::bit_cast<uint32_t>(std::get<1>(tmp)) |
            std::bit_cast<uint32_t>(std::get<2>(tmp)) |
            std::bit_cast<uint32_t>(std::get<3>(tmp))) == 0;
    }
#endif

#if defined(HI_HAS_SSE3)
    [[nodiscard]] hi_force_inline reg hadd(reg a, reg b) const noexcept { return _mm_hadd_ps(a, b); }
#else
    [[nodiscard]] hi_force_inline reg hadd(reg a, reg b) const noexcept
    {
        auto const tmp1 = _mm_shuffle_ps(a, b, 0b10'00'10'00);
        auto const tmp2 = _mm_shuffle_ps(a, b, 0b11'01'11'01);
        return _mm_add_ps(tmp1, tmp2);
    }
#endif

#if defined(HI_HAS_SSE3)
    [[nodiscard]] hi_force_inline reg hsub(reg a, reg b) const noexcept { return _mm_hsubd_ps(a, b); }
#else
    [[nodiscard]] hi_force_inline reg hsub(reg a, reg b) const noexcept
    {
        auto const tmp1 = _mm_shuffle_ps(a, b, 0b10'00'10'00);
        auto const tmp2 = _mm_shuffle_ps(a, b, 0b11'01'11'01);
        return _mm_sub_ps(tmp1, tmp2);
    }
#endif

    template<int... Indices>
    [[nodiscard]] constexpr size_t make_indices_imm() const noexcept
    {
        static_assert(sizeof...(Indices) == 4);

        constexpr auto indices = std::array{Indices...};
        for (size_t i = 0; i != 4; ++i) {
            auto index = indices[i] < 0 ? i : indices[i];
            r |= index << (i * 2);
        }
        return r;
    }

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg shuffle(reg a) const noexcept { return _mm_shuffle_ps(a, a, make_indices_imm<Indices...>{}); }

#if defined(HI_HAS_SSE4_1)
    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg blend(reg a, reg b) const noexcept { return _mm_blend_ps(a, b, Mask); }
#else
    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg blend(reg a, reg b) const noexcept
    {
        auto const lo = _mm_packlo_ps(a, b);
        auto const hi = _mm_packhi_ps(a, b);
        constexpr indices =
            (Mask & 0b0001 ? 0b00'00'00'00 : 0b00'00'00'01) |
            (Mask & 0b0010 ? 0b00'00'10'00 : 0b00'00'11'00) |
            (Mask & 0b0100 ? 0b00'00'00'00 : 0b00'01'00'00) |
            (Mask & 0b1000 ? 0b10'00'00'00 : 0b11'00'00'00);
        return _mm_shuffle_ps(lo, hi, indices);
    }
#endif

    [[nodiscard]] hi_force_inline reg sum(reg x_y_z_w) const noexcept
    {
        auto const y_x_w_z = _mm_shuffle_ps(x_y_z_w, x_y_z_w, 0b10'11'00'01);
        auto const xy_yx_zw_wz = _mm_add_ps(x_y_z_w, y_x_w_z);
        auto const zw_wz_w_z = _mm_movehl_ps(y_x_w_z, xy_yx_zw_wz);
        auto const xyzw_0_0_0 = _mm_add_ss(xy_yx_zw_wz, zw_wz_w_z);
        return _mm_shuffle_ps(xyzw_0_0_0, xyzw_0_0_0, 0);
    }

#if defined(HI_HAS_SSE4_1)
    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg dot_product(reg a, reg b) const noexcept { return _mm_dp_ps(a, b, Mask); }
#else
    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg dot_product(reg a, reg b) const noexcept {
        auto const multiplied = blend<Mask>(set_zero(), mul(a, b));
        return blend<Mask>(set_zero(), sum(multiplied));
    }
#endif

    // clang-format on
};
#endif

}}

