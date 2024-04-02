// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "array_intrinsic.hpp"
#include "float_to_half.hpp"
#include "half.hpp"
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

#if defined(HI_HAS_SSE2)
template<>
struct array_intrinsic<half, 4> {
    using value_type = half;
    using register_type = __m128i;
    using array_type = std::array<half, 4>;

    /** Load an array into a register.
     */
    [[nodiscard]] hi_force_inline static register_type L(array_type a) noexcept
    {
        return _mm_set_epi64x(0, std::bit_cast<int64_t>(a));
    }

    /** Store a register into an array.
     */
    [[nodiscard]] hi_force_inline static array_type S(register_type a) noexcept
    {
        return std::bit_cast<array_type>(_mm_cvtsi128_si64(a));
    }


    [[nodiscard]] hi_force_inline static array_type convert(std::array<float, 4> a) noexcept
    {
        if (has_f16c()) {
            return std::bit_cast<array_type>(float_to_half_f16c(a));
        } else {
            return std::bit_cast<array_type>(float_to_half_sse2(a));
        }
    }

    [[nodiscard]] hi_force_inline static array_type undefined() noexcept
    {
        return S(_mm_undefined_si128());
    }

    [[nodiscard]] hi_force_inline static array_type set_zero() noexcept
    {
        return S(_mm_setzero_si128());
    }

    [[nodiscard]] hi_force_inline static array_type set_all_ones() noexcept
    {
        return S(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
    }

    [[nodiscard]] hi_force_inline static array_type set_one() noexcept
    {
        auto const ones = _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128());
        return S(_mm_srli_epi16(_mm_slli_epi16(ones, 12), 2));
    }

    [[nodiscard]] hi_force_inline static array_type inv(array_type a) noexcept
    {
        return _xor(set_all_ones(), a);
    }

    [[nodiscard]] hi_force_inline static bool test(array_type a, array_type b) noexcept
    {
#if defined(HI_HAS_SSE4_1)
        return static_cast<bool>(_mm_testz_si128(L(a), L(b)));
#else
        return _mm_movemask_epi8(_mm_cmpeq_epi32(_mm_and_si128(L(a), L(b)), _mm_setzero_si128())) == 0xffff;
#endif
    }

    [[nodiscard]] hi_force_inline static array_type _or(array_type a, array_type b) noexcept
    {
        return S(_mm_or_si128(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type _and(array_type a, array_type b) noexcept
    {
        return S(_mm_and_si128(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type _xor(array_type a, array_type b) noexcept
    {
        return S(_mm_xor_si128(L(a), L(b)));
    }
    
    [[nodiscard]] hi_force_inline static array_type andnot(array_type a, array_type b) noexcept
    {
        return S(_mm_andnot_si128(L(a), L(b)));
    }

    [[nodiscard]] hi_force_inline static array_type sll(array_type a, unsigned int b) noexcept
    {
        auto const b_ = _mm_set_epi32(0, 0, 0, b);
        return S(_mm_sll_epi16(L(a), b_));
    }

    [[nodiscard]] hi_force_inline static array_type srl(array_type a, unsigned int b) noexcept
    {
        auto const b_ = _mm_set_epi32(0, 0, 0, b);
        return S(_mm_srl_epi16(L(a), b_));
    }

    [[nodiscard]] hi_force_inline static array_type sra(array_type a, unsigned int b) noexcept
    {
        auto const b_ = _mm_set_epi32(0, 0, 0, b);
        return S(_mm_sra_epi16(L(a), b_)));
    }

#if defined(HI_HAS_SSE4_1)
    template<size_t Mask>
    [[nodiscard]] hi_force_inline static array_type blend(array_type a, array_type b) noexcept
    {
        return S(_mm_blend_epi16(L(a), L(b), Mask));
    }
#endif
};
#endif

} // namespace v1
} // namespace v1
