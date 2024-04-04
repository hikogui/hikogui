// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "array_intrinsic.hpp"
#include "macros.hpp"
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

hi_export_module(hikocpu : array_intrinsic_f32x4);

hi_export namespace hi {
inline namespace v1 {

#if defined(HI_HAS_SSE2)
template<>
struct array_intrinsic<double, 2> {
    using value_type = double;
    using register_type = __m128d;
    using array_type = std::array<double, 2>;

    /** Load an array into a register.
     */
    [[nodiscard]] hi_force_inline static register_type L(array_type a) noexcept
    {
        return _mm_loadu_pd(a.data());
    }

    /** Store a register into an array.
     */
    [[nodiscard]] hi_force_inline static array_type S(register_type a) noexcept
    {
        auto r = array_type{};
        _mm_storeu_pd(r.data(), a);
        return r;
    }

    [[nodiscard]] hi_force_inline static array_type undefined() noexcept
    {
        return S(_mm_undefined_pd());
    }

    [[nodiscard]] hi_force_inline static array_type set(float a, float b) noexcept
    {
        return S(_mm_set_pd(b, a));
    }

    [[nodiscard]] hi_force_inline static array_type set(float a) noexcept
    {
        return S(_mm_set_pd(0.0, a));
    }

    [[nodiscard]] hi_force_inline static array_type set_zero() noexcept
    {
        return S(_mm_setzero_pd());
    }

    [[nodiscard]] hi_force_inline static array_type set_all_ones() noexcept
    {
        return S(_mm_castsi128_pd(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128())));
    }

    [[nodiscard]] hi_force_inline static array_type set_one() noexcept
    {
        auto const ones = _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128());
        return S(_mm_castsi128_pd(_mm_srli_epi64(_mm_slli_epi64(ones, 54), 2)));
    }

    template<size_t I>
    [[nodiscard]] hi_force_inline static float get(array_type a) noexcept
    {
        static_assert(I < 2);
        if constexpr (I == 0) {
            return _mm_cvtsd_f64(L(a));
        } else {
            return _mm_cvtsd_f64(_mm_shuffle_pd(L(a), L(a), I));
        }
    }

    [[nodiscard]] hi_force_inline static array_type broadcast(float a) noexcept
    {
        return S(_mm_set1_pd(a));
    }

    [[nodiscard]] hi_force_inline static array_type broadcast(array_type a) noexcept
    {
        return S(_mm_shuffle_pd(L(a), L(a), 0));
    }

    [[nodiscard]] hi_force_inline static array_type set_mask(std::size_t mask) noexcept
    {
        // clang-format off
        auto const tmp = _mm_set_epi32(
            static_cast<int32_t>(mask) << 30,
            static_cast<int32_t>(mask) << 30,
            static_cast<int32_t>(mask) << 31,
            static_cast<int32_t>(mask) << 31);
        // clang-format on

        return S(_mm_castsi128_pd(_mm_srai_epi32(tmp, 31)));
    }

    /** Store a register as a mask-integer.
     */
    [[nodiscard]] hi_force_inline static std::size_t get_mask(array_type a) noexcept
    {
        return _mm_movemask_pd(L(a));
    }

    [[nodiscard]] hi_force_inline static array_type neg(array_type a) noexcept
    {
        return S(_mm_sub_pd(_mm_setzero_pd(), L(a)));
    }

    template<std::size_t Mask>
    [[nodiscard]] hi_force_inline constexpr static array_type neg_mask(array_type a) noexcept
    {
        if constexpr (Mask == 0) {
            return a;
        } else if constexpr (Mask == 0b11) {
            return S(_mm_sub_pd(_mm_setzero_pd(), L(a)));
#if defined(HI_HAS_SSE3)
        } else if constexpr (Mask == 0b01) {
            return S(_mm_addsub_pd(_mm_setzero_pd(), L(a)));
#endif
        } else {
            auto const tmp = _mm_sub_pd(_mm_setzero_pd(), L(a));
            return blend<Mask>(a, S(tmp));
        }
    }

    [[nodiscard]] hi_force_inline static array_type inv(array_type a) noexcept
    {
        return _xor(set_all_ones(), a);
    }

    [[nodiscard]] hi_force_inline static array_type sqrt(array_type a) noexcept
    {
        return S(_mm_sqrt_pd(L(a)));
    }

#if defined(HI_HAS_SSE4_1)
    [[nodiscard]] hi_force_inline static array_type round(array_type a) noexcept
    {
        return S(_mm_round_pd(L(a), _MM_FROUND_CUR_DIRECTION));
    }
    
    [[nodiscard]] hi_force_inline static array_type floor(array_type a) noexcept
    {
        return S(_mm_floor_pd(L(a)));
    }

    [[nodiscard]] hi_force_inline static array_type ceil(array_type a) noexcept
    {
        return S(_mm_ceil_pd(L(a)));
    }
#endif

    [[nodiscard]] hi_force_inline static array_type add(array_type a, array_type b) noexcept
    {
        return S(_mm_add_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type sub(array_type a, array_type b) noexcept
    {
        return S(_mm_sub_pd(L(a), L(b)));
    }

    template<std::size_t Mask>
    [[nodiscard]] hi_force_inline constexpr static array_type addsub_mask(array_type a, array_type b) noexcept
    {
        if constexpr (Mask == 0) {
            return sub(a, b);
        } else if constexpr (Mask == 0b11) {
            return add(a, b);
#if defined(HI_HAS_SSE3)
        } else if constexpr (Mask == 0b10) {
            return S(_mm_addsub_pd(L(a), L(b)));
#endif
        } else {
            return blend<Mask>(sub(a, b), add(a, b));
        }
    }

    [[nodiscard]] hi_force_inline static array_type mul(array_type a, array_type b) noexcept
    {
        return S(_mm_mul_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type div(array_type a, array_type b) noexcept
    {
        return S(_mm_div_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type eq(array_type a, array_type b) noexcept
    {
        return S(_mm_cmpeq_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type ne(array_type a, array_type b) noexcept
    {
        return S(_mm_cmpneq_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type lt(array_type a, array_type b) noexcept
    {
        return S(_mm_cmplt_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type gt(array_type a, array_type b) noexcept
    {
        return S(_mm_cmpgt_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type le(array_type a, array_type b) noexcept
    {
        return S(_mm_cmple_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type ge(array_type a, array_type b) noexcept
    {
        return S(_mm_cmpge_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static bool test(array_type a, array_type b) noexcept
    {
#if defined(HI_HAS_SSE4_1)
        return static_cast<bool>(_mm_testz_si128(_mm_castpd_si128(L(a)), _mm_castpd_si128(L(b))));
#else
        return _mm_movemask_epi8(_mm_cmpeq_epi32(_mm_castpd_si128(_mm_and_pd(L(a), L(b))), _mm_setzero_si128())) == 0xffff;
#endif
    }

    [[nodiscard]] hi_force_inline static array_type max(array_type a, array_type b) noexcept
    {
        return S(_mm_max_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type min(array_type a, array_type b) noexcept
    {
        return S(_mm_min_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type clamp(array_type v, array_type lo, array_type hi) noexcept
    {
        return S(_mm_min_pd(_mm_max_pd(L(v), L(lo)), L(hi)));
    }

    [[nodiscard]] hi_force_inline static array_type _or(array_type a, array_type b) noexcept
    {
        return S(_mm_or_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type _and(array_type a, array_type b) noexcept
    {
        return S(_mm_and_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type _xor(array_type a, array_type b) noexcept
    {
        return S(_mm_xor_pd(L(a), L(b)));
    }
    
    [[nodiscard]] hi_force_inline static array_type andnot(array_type a, array_type b) noexcept
    {
        return S(_mm_andnot_pd(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type sll(array_type a, unsigned int b) noexcept
    {
        auto const b_ = _mm_set_epi32(0, 0, 0, b);
        return S(_mm_castsi128_pd(_mm_sll_epi64(_mm_castpd_si128(L(a)), b_)));
    }

    [[nodiscard]] hi_force_inline static array_type srl(array_type a, unsigned int b) noexcept
    {
        auto const b_ = _mm_set_epi32(0, 0, 0, b);
        return S(_mm_castsi128_pd(_mm_srl_epi64(_mm_castpd_si128(L(a)), b_)));
    }

    [[nodiscard]] hi_force_inline static array_type sra(array_type a, unsigned int b) noexcept
    {
        auto const b_ = _mm_set_epi32(0, 0, 0, b);
        return S(_mm_castsi128_pd(_mm_sra_epi64(_mm_castpd_si128(L(a)), b_)));
    }

#if defined(HI_HAS_SSE3)
    [[nodiscard]] hi_force_inline static array_type hadd(array_type a, array_type b) noexcept
    {
        return S(_mm_hadd_pd(L(a), L(b)));
    }
#endif

#if defined(HI_HAS_SSE3)
    [[nodiscard]] hi_force_inline static array_type hsub(array_type a, array_type b) noexcept
    {
        return S(_mm_hsub_pd(L(a), L(b)));
    }
#endif

    template<int... Indices>
    [[nodiscard]] constexpr static unsigned int _make_indices_imm() noexcept
    {
        static_assert(sizeof...(Indices) == 2);

        constexpr auto indices = std::array{Indices...};
        auto r = 0U;
        for (size_t i = 0; i != 2; ++i) {
            auto const index = indices[i] < 0 ? i : indices[i];
            r |= index << (i * 2);
        }
        return r;
    }

    template<int... Indices>
    [[nodiscard]] hi_force_inline static array_type shuffle(array_type a) noexcept
    {
        return S(_mm_shuffle_pd(L(a), L(a), _make_indices_imm<Indices...>()));
    }

    template<size_t Mask>
    [[nodiscard]] hi_force_inline static array_type blend(array_type a, array_type b) noexcept
    {
#if defined(HI_HAS_SSE4_1)
        return S(_mm_blend_pd(L(a), L(b), Mask));
#else
        auto const lo = _mm_unpacklo_pd(L(a), L(b));
        auto const hi = _mm_unpackhi_pd(L(a), L(b));
        return S(_mm_shuffle_pd(lo, hi, Mask));
#endif
    }

    [[nodiscard]] hi_force_inline static array_type sum(array_type a) noexcept
    {
        auto const tmp = _mm_shuffle_pd(L(a), L(a), 0b01);
        return S(_mm_add_pd(L(a), tmp));
    }

    template<size_t Mask>
    [[nodiscard]] hi_force_inline static array_type dot(array_type a, array_type b) noexcept
    {
#if defined(HI_HAS_SSE4_1)
        return S(_mm_dp_pd(L(a), L(b), (Mask << 2) | 0b11));
#else
        auto const multiplied = blend<Mask>(set_zero(), mul(a, b));
        return sum(multiplied);
#endif
    }
};
#endif

} // namespace v1
} // namespace v1
