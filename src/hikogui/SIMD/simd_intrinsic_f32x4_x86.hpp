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

hi_export namespace hi {
inline namespace v1 {

#if defined(HI_HAS_SSE)
template<>
struct simd_intrinsic<float, 4> {
    using value_type = float;
    using register_type = __m128;
    using array_type = std::array<float, 4>;

    // clang-format off
    /** Load an array into a register.
     */
    [[nodiscard]] hi_force_inline static register_type L(array_type a) noexcept { return _mm_loadu_ps(a.data()); }

    /** Store a register into an array.
     */
    [[nodiscard]] hi_force_inline static array_type S(register_type a) noexcept { auto r = array_type{}; _mm_storeu_ps(r.data(), a); return r; }

    /** Store a register as a mask-integer.
     */
    [[nodiscard]] hi_force_inline static size_t SM(array_type a) noexcept { return _mm_movemask_ps(a); }

    [[nodiscard]] hi_force_inline static array_type undefined() noexcept { return S(_mm_undefined_ps()); }
    [[nodiscard]] hi_force_inline static array_type set(float a, float b, float c, float d) noexcept { return S(_mm_set_ps(d, c, b, a)); }
    [[nodiscard]] hi_force_inline static array_type set(float a) noexcept { return S(_mm_set_ps(0.0f, 0.0f, 0.0f, a)); }
    [[nodiscard]] hi_force_inline static array_type set_zero() noexcept { return S(_mm_setzero_ps()); }
    [[nodiscard]] hi_force_inline static array_type set_all_ones() noexcept {
#if defined(HI_HAS_SSE2)
        return S(_mm_castsi128_ps(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128())));
#else
        return S(_mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps()));
#endif
    }

    [[nodiscard]] hi_force_inline static array_type set_one() noexcept {
#if defined(HI_HAS_SSE2)
        auto const ones = _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
        return S(_mm_castsi128_ps(_mm_srli_epi32(_mm_slli_epi32(ones, 25), 2)));
#else
        return S(_mm_set1_ps(1.0f));
#endif
    }

    template<size_t I>
    [[nodiscard]] hi_force_inline static float get(array_type a) noexcept
    {
        if constexpr (I == 0) {
            return _mm_cvtss_f32(L(a));
        } else {
#if defined(HI_HAS_SSE4_1)
            return _mm_extract_ps(L(a), I);
#else
            return _mm_cvtss_f32(_mm_shuffle_ps(L(a), L(a), 0b11'10'01'00 | I));
#endif
        }
    }

#if defined(HI_HAS_SSE2)
    template<typename T, size_t N>
    [[nodiscard]] hi_force_inline static array_type convert(__m128i) noexcept = delete;
    template<>
    [[nodiscard]] hi_force_inline static array_type convert<int32_t, 4>(__m128i a) noexcept { return _mm_cvtepi32_ps(a); }
    template<>
    [[nodiscard]] hi_force_inline static array_type convert<double, 2>(__m128d a, __m128d b) noexcept {
        auto const a_ = _mm_cvtpd_ps(a);
        auto const b_ = _mm_cvtpd_ps(b);
        return _mm_blend_ps(a_, b_, 0b01'00'01'00);
    }
#endif

    [[nodiscard]] hi_force_inline static array_type broadcast(float a) noexcept { return S(_mm_set1_ps(a)); }
    [[nodiscard]] hi_force_inline static array_type broadcast(array_type a) noexcept { return S(_mm_shuffle_ps(L(a), L(a), 0)); }

    [[nodiscard]] hi_force_inline static array_type neg(array_type a) noexcept { return S(_mm_sub_ps(_mm_setzero_ps(), L(a)); }
    [[nodiscard]] hi_force_inline static array_type inv(array_type a) noexcept { return S(_mm_xor_ps(set_all_ones(), a)); }

    [[nodiscard]] hi_force_inline static array_type rcp(array_type a) noexcept { return S(_mm_rcp_ps(L(a))); }
    [[nodiscard]] hi_force_inline static array_type sqrt(array_type a) noexcept { return S(_mm_sqrt_ps(L(a))); }
    [[nodiscard]] hi_force_inline static array_type rsqrt(array_type a) noexcept { return S(_mm_rsqrt_ps(L(a))); }

#if defined(HI_HAS_SSE2)
    [[nodiscard]] hi_force_inline static array_type round(array_type a) noexcept {
#if defined(HI_HAS_SSE4_1)
        return S(_mm_round_ps(L(a), _MM_FROUND_CUR_DIRECTION));
#else
        auto const a_ = L(a);
        auto const rounded = _mm_cvtepi32_ps(_mm_cvtps_epi32(a_));
        auto const check_max = _mm_cmple_ps(a_, _mm_set1_ps(static_cast<float>(std::numeric_limits<int32_t>::max())));
        auto const check_min = _mm_cmpge_ps(a_, _mm_set1_ps(static_cast<float>(std::numeric_limits<int32_t>::min())));
        auto const check_bounds = _mm_and_ps(check_max, check_min);

        auto const good_rounded = _mm_and_ps(check_bounds, rounded);
        auto const good_a = _mm_andnot_ps(check_bounds, a_);
        return S(_mm_or_ps(good_rounded, good_a));
#endif
    }
#endif
#if defined(HI_HAS_SSE4_1)
    [[nodiscard]] hi_force_inline static array_type floor(array_type a) noexcept { return S(_mm_floor_ps(L(a))); }
    [[nodiscard]] hi_force_inline static array_type ceil(array_type a) noexcept { return S(_mm_ceil_ps(L(a))); }
#endif

    [[nodiscard]] hi_force_inline static array_type add(array_type a, array_type b) noexcept { return S(_mm_add_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type sub(array_type a, array_type b) noexcept { return S(_mm_sub_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type mul(array_type a, array_type b) noexcept { return S(_mm_mul_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type div(array_type a, array_type b) noexcept { return S(_mm_div_ps(L(a), L(b))); }

    [[nodiscard]] hi_force_inline static array_type eq(array_type a, array_type b) noexcept { return S(_mm_cmpeq_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type ne(array_type a, array_type b) noexcept { return S(_mm_cmpneq_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type lt(array_type a, array_type b) noexcept { return S(_mm_cmplt_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type gt(array_type a, array_type b) noexcept { return S(_mm_cmpgt_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type le(array_type a, array_type b) noexcept { return S(_mm_cmple_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type ge(array_type a, array_type b) noexcept { return S(_mm_cmpge_ps(L(a), L(b))); }

    [[nodiscard]] hi_force_inline static bool test(array_type a, array_type b) noexcept
    {
#if defined(HI_HAS_SSE4_1)
        return static_cast<bool>(_mm_testz_si128(_mm_castps_si128(L(a)), _mm_castps_si128(L(b))));
#elif defined(HI_HAS_SSE2)
        return _mm_movemask_epi8(_mm_cmpeq_epi32(_mm_castps_si128(_mm_and_ps(L(a), L(b))), _mm_setzero_si128())) == 0xffff;
#else
        auto tmp = std::array<float, 4>{};
        _mm_store_ps(tmp.data(), _mm_and_ps(L(a), L(b)));

        return (
            std::bit_cast<uint32_t>(std::get<0>(tmp)) |
            std::bit_cast<uint32_t>(std::get<1>(tmp)) |
            std::bit_cast<uint32_t>(std::get<2>(tmp)) |
            std::bit_cast<uint32_t>(std::get<3>(tmp))) == 0;
#endif
    }

    [[nodiscard]] hi_force_inline static array_type max(array_type a, array_type b) noexcept { return S(_mm_max_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type min(array_type a, array_type b) noexcept { return S(_mm_min_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type clamp(array_type v, array_type lo, array_type hi) noexcept {
        return S(_mm_min_ps(_mm_max_ps(L(v), L(lo)), L(hi));
    }

    [[nodiscard]] hi_force_inline static array_type _or(array_type a, array_type b) noexcept { return S(_mm_or_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type _and(array_type a, array_type b) noexcept { return S(_mm_and_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type _xor(array_type a, array_type b) noexcept { return S(_mm_xor_ps(L(a), L(b))); }
    [[nodiscard]] hi_force_inline static array_type andnot(array_type a, array_type b) noexcept { return S(_mm_andnot_ps(L(a), L(b))); }

    [[nodiscard]] hi_force_inline static array_type int_to_mask(size_t mask) noexcept
    {
#if defined(HI_HAS_SSE2)
        auto a = set(static_cast<int32_t>(mask) << 31);
        auto b = set(static_cast<int32_t>(mask) << 30);
        auto c = set(static_cast<int32_t>(mask) << 29);
        auto d = set(static_cast<int32_t>(mask) << 28);
        auto lo = _mm_castsi128_ps(_mm_unpacklo_epi32(a, b));
        auto hi = _mm_castsi128_ps(_mm_unpacklo_epi32(c, d));

        auto tmp = _mm_castps_si128(_mm_shuffle_ps(lo, hi, 0b01'00'01'00));
        return S(_mm_castsi128_ps(_mm_srai_epi32(tmp, 31)));
#else
        auto mask_ = static_cast<int64_t>(mask) << 31;

        auto r = std::array<int32_t, 4>{};
        std::get<0>(r) = static_cast<int32_t>(mask_) >> 31;
        mask_ >>= 1;
        std::get<1>(r) = static_cast<int32_t>(mask_) >> 31;
        mask_ >>= 1;
        std::get<2>(r) = static_cast<int32_t>(mask_) >> 31;
        mask_ >>= 1;
        std::get<3>(r) = static_cast<int32_t>(mask_) >> 31;
        return S(_mm_loadu_ps(reinterpret_cast<float const *>(r.data())));
#endif
    }

    [[nodiscard]] hi_force_inline static array_type hadd(array_type a, array_type b) noexcept {
#if defined(HI_HAS_SSE3)
        return S(_mm_hadd_ps(L(a), L(b)));
#else
        auto const a_ = L(a);
        auto const b_ = L(b);
        auto const tmp1 = _mm_shuffle_ps(a_, b_, 0b10'00'10'00);
        auto const tmp2 = _mm_shuffle_ps(a_, b_, 0b11'01'11'01);
        return S(_mm_add_ps(tmp1, tmp2));
#endif
    }

    [[nodiscard]] hi_force_inline static array_type hsub(array_type a, array_type b) noexcept {
#if defined(HI_HAS_SSE3)
        return S(_mm_hsubd_ps(L(a), L(b)));
#else
        auto const a_ = L(a);
        auto const b_ = L(b);
        auto const tmp1 = _mm_shuffle_ps(a_, b_, 0b10'00'10'00);
        auto const tmp2 = _mm_shuffle_ps(a_, b_, 0b11'01'11'01);
        return S(_mm_sub_ps(tmp1, tmp2));
#endif
    }

    template<int... Indices>
    [[nodiscard]] constexpr size_t make_indices_imm() noexcept
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
    [[nodiscard]] hi_force_inline static array_type shuffle(array_type a) noexcept { return _mm_shuffle_ps(a, a, make_indices_imm<Indices...>{}); }

    template<size_t Mask>
    [[nodiscard]] hi_force_inline static array_type blend(array_type a, array_type b) noexcept {
#if defined(HI_HAS_SSE4_1)
        return _mm_blend_ps(a, b, Mask);
#else
        auto const lo = _mm_packlo_ps(a, b);
        auto const hi = _mm_packhi_ps(a, b);
        constexpr indices =
            (Mask & 0b0001 ? 0b00'00'00'00 : 0b00'00'00'01) |
            (Mask & 0b0010 ? 0b00'00'10'00 : 0b00'00'11'00) |
            (Mask & 0b0100 ? 0b00'00'00'00 : 0b00'01'00'00) |
            (Mask & 0b1000 ? 0b10'00'00'00 : 0b11'00'00'00);
        return _mm_shuffle_ps(lo, hi, indices);
#endif
    }

    [[nodiscard]] hi_force_inline static array_type sum(array_type x_y_z_w) noexcept
    {
        auto const y_x_w_z = _mm_shuffle_ps(x_y_z_w, x_y_z_w, 0b10'11'00'01);
        auto const xy_yx_zw_wz = _mm_add_ps(x_y_z_w, y_x_w_z);
        auto const zw_wz_w_z = _mm_movehl_ps(y_x_w_z, xy_yx_zw_wz);
        auto const xyzw_0_0_0 = _mm_add_ss(xy_yx_zw_wz, zw_wz_w_z);
        return _mm_shuffle_ps(xyzw_0_0_0, xyzw_0_0_0, 0);
    }

    template<size_t Mask>
    [[nodiscard]] hi_force_inline static array_type dot_product(array_type a, array_type b) noexcept {
#if defined(HI_HAS_SSE4_1)
        return _mm_dp_ps(a, b, Mask);
#else
        auto const multiplied = blend<Mask>(set_zero(), mul(a, b));
        return blend<Mask>(set_zero(), sum(multiplied));
#endif
    }

    // clang-format on
};
#endif

} // namespace v1
} // namespace v1
