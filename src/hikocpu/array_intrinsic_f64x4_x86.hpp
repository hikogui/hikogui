// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "array_intrinsic.hpp"
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

hi_export_module(hikogui.SIMD.array_intrinsic_f32x4);

hi_export namespace hi {
inline namespace v1 {

#if defined(HI_HAS_AVX)
template<>
struct array_intrinsic<double, 4> {
    using value_type = double;
    using register_type = __m256d;
    using array_type = std::array<double, 4>;

    /** Load an array into a register.
     */
    [[nodiscard]] hi_force_inline static register_type L(array_type a) noexcept
    {
        return _mm256_loadu_pd(a.data());
    }

    /** Store a register into an array.
     */
    [[nodiscard]] hi_force_inline static array_type S(register_type a) noexcept
    {
        auto r = array_type{};
        _mm256_storeu_pd(r.data(), a);
        return r;
    }

    [[nodiscard]] hi_force_inline static array_type undefined() noexcept
    {
        return S(_mm256_undefined_pd());
    }

    [[nodiscard]] hi_force_inline static array_type set(double a, double b, double c, double d) noexcept
    {
        return S(_mm256_set_pd(d, c, b, a));
    }

    [[nodiscard]] hi_force_inline static array_type set(double a) noexcept
    {
        return S(_mm256_set_pd(0.0, 0.0, 0.0, a));
    }

    [[nodiscard]] hi_force_inline static array_type set_zero() noexcept
    {
        return S(_mm256_setzero_pd());
    }

    [[nodiscard]] hi_force_inline static array_type set_all_ones() noexcept
    {
#if defined(HI_HAS_AVX2)
        return S(_mm256_castsi256_pd(_mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256())));
#else
        return S(_mm256_cmpeq_pd(_mm256_setzero_pd(), _mm256_setzero_pd()));
#endif
    }

    [[nodiscard]] hi_force_inline static array_type set_one() noexcept
    {
#if defined(HI_HAS_AVX2)
        auto const ones = _mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256());
        return S(_mm256_castsi256_pd(_mm256_srli_epi32(_mm256_slli_epi32(ones, 25), 2)));
#else
        return S(_mm256_set1_pd(1.0f));
#endif
    }

    template<size_t I>
    [[nodiscard]] hi_force_inline static double get(array_type a) noexcept
    {
        static_assert(I < 4);

        if constexpr (I == 0) {
            return _mm256_cvtsd_f64(L(a));
        } else if constexpr (I == 1) {
            return _mm256_cvtsd_f64(_mm256_shuffle_pd(L(a), L(a), 0b1));
        } else {
            auto const tmp = _mm256_extractf128_pd(L(a), 0b1);
            if constexpr (I == 2) {
                return _mm_cvtsd_f64(tmp);
            } else {
                return _mm_cvtsd_f64(_mm_permute_pd(tmp, 0b1));
            }
        }
    }

    [[nodiscard]] hi_force_inline static array_type broadcast(double a) noexcept
    {
        return S(_mm256_set1_pd(a));
    }

    [[nodiscard]] hi_force_inline static array_type broadcast(array_type a) noexcept
    {
        auto tmp = L(a);
        auto lo = _mm256_extractf128_pd(tmp, 0b0);
        tmp = _mm256_insertf128_pd(tmp, lo, 0b1);
        return S(_mm256_permute_pd(tmp, 0b0000));
    }

    /** Store a register as a mask-integer.
     */
    [[nodiscard]] hi_force_inline static std::size_t get_mask(array_type a) noexcept
    {
        return _mm256_movemask_pd(L(a));
    }

    [[nodiscard]] hi_force_inline static array_type neg(array_type a) noexcept
    {
        return S(_mm256_sub_pd(_mm256_setzero_pd(), L(a)));
    }

    template<std::size_t Mask>
    [[nodiscard]] hi_force_inline constexpr static array_type neg_mask(array_type a) noexcept
    {
        if constexpr (Mask == 0) {
            return a;
        } else if constexpr (Mask == 0b1111) {
            return S(_mm256_sub_pd(_mm256_setzero_pd(), L(a)));
#if defined(HI_HAS_SSE3)
        } else if constexpr (Mask == 0b0101) {
            return S(_mm256_addsub_pd(_mm256_setzero_pd(), L(a)));
#endif
        } else {
            auto const tmp = _mm256_sub_pd(_mm256_setzero_pd(), L(a));
            return blend<Mask>(a, S(tmp));
        }
    }

    [[nodiscard]] hi_force_inline static array_type inv(array_type a) noexcept
    {
        return _xor(set_all_ones(), a);
    }

    [[nodiscard]] hi_force_inline static array_type rcp(array_type a) noexcept
    {
        return S(_mm256_div_pd(_mm256_set1_pd(1.0), L(a)));
    }

    [[nodiscard]] hi_force_inline static array_type sqrt(array_type a) noexcept
    {
        return S(_mm256_sqrt_pd(L(a)));
    }

    [[nodiscard]] hi_force_inline static array_type rsqrt(array_type a) noexcept
    {
        return S(_mm256_div_pd(_mm256_set1_pd(1.0), _mm256_sqrt_pd(L(a))));
    }

#if defined(HI_HAS_SSE2)
    [[nodiscard]] hi_force_inline static array_type round(array_type a) noexcept
    {
#if defined(HI_HAS_SSE4_1)
        return S(_mm256_round_pd(L(a), _MM_FROUND_CUR_DIRECTION));
#else
        auto const a_ = L(a);
        auto const rounded = _mm256_cvtepi32_pd(_mm256_cvtps_epi32(a_));
        auto const check_max = _mm256_cmple_pd(a_, _mm256_set1_pd(static_cast<float>(std::numeric_limits<int32_t>::max())));
        auto const check_min = _mm256_cmpge_pd(a_, _mm256_set1_pd(static_cast<float>(std::numeric_limits<int32_t>::min())));
        auto const check_bounds = _mm256_and_pd(check_max, check_min);

        auto const good_rounded = _mm256_and_pd(check_bounds, rounded);
        auto const good_a = _mm256_andnot_pd(check_bounds, a_);
        return S(_mm256_or_pd(good_rounded, good_a));
#endif
    }
#endif

#if defined(HI_HAS_SSE4_1)
    [[nodiscard]] hi_force_inline static array_type floor(array_type a) noexcept
    {
        return S(_mm256_floor_pd(L(a)));
    }

    [[nodiscard]] hi_force_inline static array_type ceil(array_type a) noexcept
    {
        return S(_mm256_ceil_pd(L(a)));
    }
#endif

    [[nodiscard]] hi_force_inline static array_type add(array_type a, array_type b) noexcept
    {
        return S(_mm256_add_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type sub(array_type a, array_type b) noexcept
    {
        return S(_mm256_sub_pd(L(a), L(b)));
    }

    template<std::size_t Mask>
    [[nodiscard]] hi_force_inline constexpr static array_type addsub_mask(array_type a, array_type b) noexcept
    {
        if constexpr (Mask == 0) {
            return sub(a, b);
        } else if constexpr (Mask == 0b1111) {
            return add(a, b);
#if defined(HI_HAS_SSE3)
        } else if constexpr (Mask == 0b1010) {
            return S(_mm256_addsub_pd(L(a), L(b)));
#endif
        } else {
            return blend<Mask>(sub(a, b), add(a, b));
        }
    }

    [[nodiscard]] hi_force_inline static array_type mul(array_type a, array_type b) noexcept
    {
        return S(_mm256_mul_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type div(array_type a, array_type b) noexcept
    {
        return S(_mm256_div_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type eq(array_type a, array_type b) noexcept
    {
        return S(_mm256_cmp_pd(L(a), L(b), _CMP_EQ_OS));
    }

    [[nodiscard]] hi_force_inline static array_type ne(array_type a, array_type b) noexcept
    {
        return S(_mm256_cmp_pd(L(a), L(b), _CMP_NEQ_OS));
    }

    [[nodiscard]] hi_force_inline static array_type lt(array_type a, array_type b) noexcept
    {
        return S(_mm256_cmp_pd(L(a), L(b), _CMP_LT_OS));
    }

    [[nodiscard]] hi_force_inline static array_type gt(array_type a, array_type b) noexcept
    {
        return S(_mm256_cmp_pd(L(a), L(b), _CMP_GT_OS));
    }

    [[nodiscard]] hi_force_inline static array_type le(array_type a, array_type b) noexcept
    {
        return S(_mm256_cmp_pd(L(a), L(b), _CMP_LE_OS));
    }

    [[nodiscard]] hi_force_inline static array_type ge(array_type a, array_type b) noexcept
    {
        return S(_mm256_cmp_pd(L(a), L(b), _CMP_GE_OS));
    }

    [[nodiscard]] hi_force_inline static bool test(array_type a, array_type b) noexcept
    {
#if defined(HI_HAS_SSE4_1)
        return static_cast<bool>(_mm256_testz_si256(_mm256_castpd_si256(L(a)), _mm256_castpd_si256(L(b))));
#elif defined(HI_HAS_SSE2)
        return _mm256_movemask_epi8(_mm256_cmpeq_epi32(_mm256_castpd_si256(_mm256_and_pd(L(a), L(b))), _mm256_setzero_si256())) == 0xffff;
#else
        auto tmp = std::array<float, 4>{};
        _mm256_store_pd(tmp.data(), _mm256_and_pd(L(a), L(b)));

        return (std::bit_cast<uint32_t>(std::get<0>(tmp)) | std::bit_cast<uint32_t>(std::get<1>(tmp)) |
                std::bit_cast<uint32_t>(std::get<2>(tmp)) | std::bit_cast<uint32_t>(std::get<3>(tmp))) == 0;
#endif
    }

    [[nodiscard]] hi_force_inline static array_type max(array_type a, array_type b) noexcept
    {
        return S(_mm256_max_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type min(array_type a, array_type b) noexcept
    {
        return S(_mm256_min_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type clamp(array_type v, array_type lo, array_type hi) noexcept
    {
        return S(_mm256_min_pd(_mm256_max_pd(L(v), L(lo)), L(hi)));
    }

    [[nodiscard]] hi_force_inline static array_type _or(array_type a, array_type b) noexcept
    {
        return S(_mm256_or_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type _and(array_type a, array_type b) noexcept
    {
        return S(_mm256_and_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type _xor(array_type a, array_type b) noexcept
    {
        return S(_mm256_xor_pd(L(a), L(b)));
    }
    
    [[nodiscard]] hi_force_inline static array_type andnot(array_type a, array_type b) noexcept
    {
        return S(_mm256_andnot_pd(L(a), L(b)));
    }

#if defined(HI_HAS_SSE2)
    [[nodiscard]] hi_force_inline static array_type sll(array_type a, unsigned int b) noexcept
    {
        auto const b_ = _mm_set_epi32(0, 0, 0, b);
        return S(_mm256_castsi256_pd(_mm256_sll_epi32(_mm256_castpd_si256(L(a)), b_)));
    }
#endif

#if defined(HI_HAS_SSE2)
    [[nodiscard]] hi_force_inline static array_type srl(array_type a, unsigned int b) noexcept
    {
        auto const b_ = _mm_set_epi32(0, 0, 0, b);
        return S(_mm256_castsi256_pd(_mm256_srl_epi32(_mm256_castpd_si256(L(a)), b_)));
    }
#endif

#if defined(HI_HAS_SSE2)
    [[nodiscard]] hi_force_inline static array_type sra(array_type a, unsigned int b) noexcept
    {
        auto const b_ = _mm_set_epi32(0, 0, 0, b);
        return S(_mm256_castsi256_pd(_mm256_sra_epi32(_mm256_castpd_si256(L(a)), b_)));
    }
#endif

    [[nodiscard]] hi_force_inline static array_type hadd(array_type a, array_type b) noexcept
    {
#if defined(HI_HAS_SSE3)
        return S(_mm256_hadd_pd(L(a), L(b)));
#else
        auto const a_ = L(a);
        auto const b_ = L(b);
        auto const tmp1 = _mm256_shuffle_pd(a_, b_, 0b10'00'10'00);
        auto const tmp2 = _mm256_shuffle_pd(a_, b_, 0b11'01'11'01);
        return S(_mm256_add_pd(tmp1, tmp2));
#endif
    }

    [[nodiscard]] hi_force_inline static array_type hsub(array_type a, array_type b) noexcept
    {
#if defined(HI_HAS_SSE3)
        return S(_mm256_hsub_pd(L(a), L(b)));
#else
        auto const a_ = L(a);
        auto const b_ = L(b);
        auto const tmp1 = _mm256_shuffle_pd(a_, b_, 0b10'00'10'00);
        auto const tmp2 = _mm256_shuffle_pd(a_, b_, 0b11'01'11'01);
        return S(_mm256_sub_pd(tmp1, tmp2));
#endif
    }

    template<int... Indices>
    [[nodiscard]] constexpr static unsigned int _make_indices_imm() noexcept
    {
        static_assert(sizeof...(Indices) == 4);

        constexpr auto indices = std::array{Indices...};
        auto r = 0U;
        for (size_t i = 0; i != 4; ++i) {
            auto const index = indices[i] < 0 ? i : indices[i];
            r |= index << (i * 2);
        }
        return r;
    }

    template<int... Indices>
    [[nodiscard]] hi_force_inline static array_type shuffle(array_type a) noexcept
    {
        return S(_mm256_shuffle_pd(L(a), L(a), _make_indices_imm<Indices...>()));
    }

    template<size_t Mask>
    [[nodiscard]] hi_force_inline static array_type blend(array_type a, array_type b) noexcept
    {
#if defined(HI_HAS_SSE4_1)
        return S(_mm256_blend_pd(L(a), L(b), Mask));
#else
        auto const lo = _mm256_unpacklo_pd(L(a), L(b));
        auto const hi = _mm256_unpackhi_pd(L(a), L(b));
        // clang-format off
        constexpr auto indices =
            (Mask & 0b0001 ? 0b00'00'00'01U : 0b00'00'00'00U) |
            (Mask & 0b0010 ? 0b00'00'11'00U : 0b00'00'10'00U) |
            (Mask & 0b0100 ? 0b00'01'00'00U : 0b00'00'00'00U) |
            (Mask & 0b1000 ? 0b11'00'00'00U : 0b10'00'00'00U);
        // clang-format on
        return S(_mm256_shuffle_pd(lo, hi, indices));
#endif
    }

    template<size_t Mask>
    [[nodiscard]] hi_force_inline static array_type dot(array_type a, array_type b) noexcept
    {
#if defined(HI_HAS_SSE4_1)
        return S(_mm256_dp_pd(L(a), L(b), (Mask << 4) | 0b1111));
#else
        auto const multiplied = blend<Mask>(set_zero(), mul(a, b));
        return sum(multiplied);
#endif
    }
};
#endif

} // namespace v1
} // namespace v1
