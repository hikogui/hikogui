// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd_load.hpp"
#include "simd_store.hpp"
#include "../macros.hpp"
#include <cstddef>
#include <concepts>
#include <array>

hi_export_module(hikogui.simd.simd_unary_operators);

namespace hi { inline namespace v1 {

// clang-format off
template<typename T, size_t N> struct simd_not;
template<typename T, size_t N> struct simd_neg;
// clang-format on

#define HI_X(NAME, VALUE_TYPE, SIZE, XOR_OP, SET_OP) \
    template<> \
    struct NAME<VALUE_TYPE, SIZE> { \
        using array_type = std::array<VALUE_TYPE, SIZE>; \
        [[nodiscard]] hi_force_inline array_type operator()(array_type const& rhs) const noexcept \
        { \
            hilet rhs_ = simd_load<VALUE_TYPE, SIZE>(rhs); \
            return simd_store<VALUE_TYPE, SIZE>(XOR_OP(SET_OP(rhs_, rhs_), rhs_)); \
        } \
    }

#if defined(HI_HAS_SSE)
HI_X(simd_neg, float, 4, _mm_sub_ps, _mm_xor_ps);
#endif
#if defined(HI_HAS_SSE2)
HI_X(simd_neg, double, 2, _mm_sub_pd, _mm_xor_pd);

HI_X(simd_not, int64_t, 2, _mm_xor_si128, _mm_cmpeq_epi8);
HI_X(simd_neg, int64_t, 2, _mm_sub_epi64, _mm_xor_si128);

HI_X(simd_not, int32_t, 4, _mm_xor_si128, _mm_cmpeq_epi8);
HI_X(simd_neg, int32_t, 4, _mm_sub_epi32, _mm_xor_si128);

HI_X(simd_not, int16_t, 8, _mm_xor_si128, _mm_cmpeq_epi8);
HI_X(simd_neg, int16_t, 8, _mm_sub_epi16, _mm_xor_si128);

HI_X(simd_not, int8_t, 16, _mm_xor_si128, _mm_cmpeq_epi8);
HI_X(simd_neg, int8_t, 2, _mm_sub_epu8, _mm_xor_si128);

HI_X(simd_not, uint64_t, 2, _mm_xor_si128, _mm_cmpeq_epi8);
HI_X(simd_neg, uint64_t, 2, _mm_sub_epu64, _mm_xor_si128);

HI_X(simd_not, uint32_t, 4, _mm_xor_si128, _mm_cmpeq_epi8);
HI_X(simd_neg, uint32_t, 4, _mm_sub_epu32, _mm_xor_si128);

HI_X(simd_not, uint16_t, 8, _mm_xor_si128, _mm_cmpeq_epi8);
HI_X(simd_neg, uint16_t, 8, _mm_sub_epu16, _mm_xor_si128);

HI_X(simd_not, uint8_t, 16, _mm_xor_si128, _mm_cmpeq_epi8);
HI_X(simd_neg, uint8_t, 16, _mm_sub_epu8, _mm_xor_si128);
#endif
#ifdef HI_HAS_AVX
HI_X(simd_neg, double, 4, _mm256_sub_pd, _mm256_xor_pd);

HI_X(simd_neg, float, 8, _mm256_sub_ps, _mm256_xor_ps);
#endif
#if defined(HI_HAS_AVX2)
HI_X(simd_not, int64_t, 4, _mm256_xor_si256, _mm256_cmpeq_epi8);
HI_X(simd_neg, int64_t, 4, _mm256_sub_epi64, _mm256_xor_si256);

HI_X(simd_not, int32_t, 8, _mm256_xor_si256, _mm256_cmpeq_epi8);
HI_X(simd_neg, int32_t, 8, _mm256_sub_epi32, _mm256_xor_si256);

HI_X(simd_not, int16_t, 16, _mm256_xor_si256, _mm256_cmpeq_epi8);
HI_X(simd_neg, int16_t, 16, _mm256_sub_epi16, _mm256_xor_si256);

HI_X(simd_not, int8_t, 32, _mm256_xor_si256, _mm256_cmpeq_epi8);
HI_X(simd_neg, int8_t, 32, _mm256_sub_epi8, _mm256_xor_si256);

HI_X(simd_not, uint64_t, 4, _mm256_xor_si256, _mm256_cmpeq_epi8);
HI_X(simd_neg, uint64_t, 4, _mm256_sub_epu64, _mm256_xor_si256);

HI_X(simd_not, uint32_t, 8, _mm256_xor_si256, _mm256_cmpeq_epi8);
HI_X(simd_neg, uint32_t, 8, _mm256_sub_epu32, _mm256_xor_si256);

HI_X(simd_not, uint16_t, 16, _mm256_xor_si256, _mm256_cmpeq_epi8);
HI_X(simd_neg, uint16_t, 16, _mm256_sub_epu16, _mm256_xor_si256);

HI_X(simd_not, uint8_t, 32, _mm256_xor_si256, _mm256_cmpeq_epi8);
HI_X(simd_neg, uint8_t, 32, _mm256_sub_epu8, _mm256_xor_si256);
#endif
#if defined(HI_HAS_AVX512F)
HI_X(simd_neg, double, 8, _mm512_sub_pd, _mm512_xor_pd);

HI_X(simd_neg, float, 16, _mm512_sub_ps, _mm512_xor_ps);

HI_X(simd_not, int64_t, 8, _mm512_xor_si512, _mm512_cmpeq_epi8);
HI_X(simd_neg, int64_t, 8, _mm512_sub_epi64, _mm512_xor_si512);

HI_X(simd_not, int32_t, 16, _mm512_xor_si512, _mm512_cmpeq_epi8);
HI_X(simd_neg, int32_t, 16, _mm512_sub_epi32, _mm512_xor_si512);

HI_X(simd_not, int16_t, 32, _mm512_xor_si512, _mm512_cmpeq_epi8);
HI_X(simd_neg, int16_t, 32, _mm512_sub_epi16, _mm512_xor_si512);

HI_X(simd_not, int8_t, 64, _mm512_xor_si512, _mm512_cmpeq_epi8);
HI_X(simd_neg, int8_t, 64, _mm512_sub_epi8, _mm512_xor_si512);

HI_X(simd_not, uint64_t, 8, _mm512_xor_si512, _mm512_cmpeq_epi8);
HI_X(simd_neg, uint64_t, 8, _mm512_sub_epu64, _mm512_xor_si512);

HI_X(simd_not, uint32_t, 16, _mm512_xor_si512, _mm512_cmpeq_epi8);
HI_X(simd_neg, uint32_t, 16, _mm512_sub_epu32, _mm512_xor_si512);

HI_X(simd_not, uint16_t, 32, _mm512_xor_si512, _mm512_cmpeq_epi8);
HI_X(simd_neg, uint16_t, 32, _mm512_sub_epu16, _mm512_xor_si512);

HI_X(simd_not, uint8_t, 64, _mm512_xor_si512, _mm512_cmpeq_epi8);
HI_X(simd_neg, uint8_t, 64, _mm512_sub_epu8, _mm512_xor_si512);
#endif
#undef HI_X

#define HI_X(NAME, VALUE_TYPE, SIZE, XOR_OP, EQ_OP, CAST1_OP, CAST2_OP) \
    template<> \
    struct NAME<VALUE_TYPE, SIZE> { \
        using array_type = std::array<VALUE_TYPE, SIZE>; \
        [[nodiscard]] hi_force_inline array_type operator()(array_type const& rhs) const noexcept \
        { \
            hilet rhs_ = simd_load<VALUE_TYPE, SIZE>(rhs); \
            return XOR_OP(rhs_, CAST2_OP(EQ_OP(CAST1_OP(rhs_), CAST1_OP(rhs_)))); \
        } \
    }

#if defined(HI_HAS_SSE)
HI_X(simd_not, float, 4, _mm_xor_ps, _mm_cmpeq_epi8, _mm_castps_si128, _mm_castsi128_ps);
#endif
#if defined(HI_HAS_SSE2)
HI_X(simd_not, double, 2, _mm_xor_ps, _mm_cmpeq_epi8, _mm_castps_si128, _mm_castsi128_ps);
#endif
#if defined(HI_HAS_AVX)
HI_X(simd_not, float, 8, _mm256_xor_ps, _mm256_cmpeq_epi8, _mm256_castps_si256, _mm256_castsi256_ps);
HI_X(simd_not, double, 4, _mm256_xor_ps, _mm256_cmpeq_epi8, _mm256_castps_si256, _mm256_castsi256_ps);
#endif
#if defined(HI_HAS_AVX512)
HI_X(simd_not, float, 16, _mm512_xor_ps, _mm512_cmpeq_epi8, _mm512_castps_si512, _mm512_castsi512_ps);
HI_X(simd_not, double, 8, _mm512_xor_ps, _mm512_cmpeq_epi8, _mm512_castps_si512, _mm512_castsi512_ps);
#endif
#undef HI_X

}} // namespace hi::v1