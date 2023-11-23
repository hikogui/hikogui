// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <cstddef>
#include <array>
#include <cstdint>
#include <type_traits>
#include <concepts>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>
#include <wmmintrin.h>
#include <immintrin.h>

hi_export_module(hikogui.simd.binary_operators);

namespace hi { inline namespace v1 {

// clang-format off
template<typename T, size_t N> struct simd_set_all_ones;
template<typename T, size_t N> struct simd_set_one;
// clang-format on

#define X_HI_SET_ALL_ONES_FLOAT(VALUE_TYPE, SIZE, REG, CMPEQ_OP, CAST1_OP, CAST2_OP) \
    template<> \
    struct simd_set_all_ones<VALUE_TYPE, SIZE> { \
        [[nodiscard]] hi_force_inline REG operator(REG dummy)() const noexcept \
        { \
            return CAST2_OP(CMPEQ_OP(CAST1_OP(dummy), CAST1_OP(dummy))); \
        } \
    }

#define X_HI_SET_ALL_ONES_INT(VALUE_TYPE, SIZE, REG, CMPEQ_OP) \
    template<> \
    struct simd_set_all_ones<VALUE_TYPE, SIZE> { \
        [[nodiscard]] hi_force_inline REG operator(REG dummy)() const noexcept \
        { \
            return CMPEQ_OP(dummy, dummy); \
        } \
    }

#define X_HI_SET_ONE_FLOAT(VALUE_TYPE, SIZE, REG, CMPEQ_OP, SLL_OP, SHIFT_COUNT, SRL_OP, CAST1_OP, CAST2_OP) \
    template<> \
    struct simd_set_one<VALUE_TYPE, SIZE> { \
        [[nodiscard]] hi_force_inline REG operator(REG dummy)() const noexcept \
        { \
            auto r = CMPEQ_OP(CAST1_OP(dummy), CAST1_OP(dummy)); \
            r = SLL_OP(r, SHIFT_COUNT); \
            r = SRL_OP(r, 2); \
            return CAST2_OP(r); \
        } \
    }

#define X_HI_SET_ONE_INT(VALUE_TYPE, SIZE, REG, CMPEQ_OP, SRL_OP) \
    template<> \
    struct simd_set_one<VALUE_TYPE, SIZE> { \
        [[nodiscard]] hi_force_inline REG operator(REG dummy)() const noexcept \
        { \
            return SRL_OP(CMPEQ_OP(dummy, dummy), sizeof(VALUE_TYPE) * CHAR_BIT - 1); \
        } \
    }

#define X_HI_SET_ONE_INT8(VALUE_TYPE, SIZE, REG, CMPEQ_OP, ABS_OP) \
    template<> \
    struct simd_set_one<VALUE_TYPE, SIZE> { \
        [[nodiscard]] hi_force_inline REG operator(REG dummy)() const noexcept \
        { \
            return ABS_OP(CMPEQ_OP(dummy, dummy)); \
        } \
    }

#if defined(HI_HAS_SSE2)
X_HI_SET_ALL_ONES_FLOAT(double, 2, __m128d, _mm_cmpeq_epi32, _mm_castpd_si128, _mm_castsi128_pd);
X_HI_SET_ONE_FLOAT(double, 2, __m128d, _mm_cmpeq_epi32, _mm_slli_epi64, 64 - 10, _mm_srli_epi64, _mm_castpd_si128, _mm_castsi128_pd);

X_HI_SET_ALL_ONES_FLOAT(float, 4, __m128, _mm_cmpeq_epi32, _mm_castps_si128, _mm_castsi128_ps);
X_HI_SET_ONE_FLOAT(float, 4, __m128, _mm_cmpeq_epi32, _mm_slli_epi32, 32 - 7, _mm_srli_epi32, _mm_castpd_si128, _mm_castsi128_pd);

X_HI_SET_ALL_ONES_INT(int64_t, 2, __m128i, _mm_cmpeq_epi32);
X_HI_SET_ONE_INT(int64_t, 2, __m128i, _mm_cmpeq_epi32, _mm_srli_epi64);

X_HI_SET_ALL_ONES_INT(int32_t, 4, __m128i, _mm_cmpeq_epi32);
X_HI_SET_ONE_INT(int32_t, 4, __m128i, _mm_cmpeq_epi32, _mm_srli_epi32);

X_HI_SET_ALL_ONES_INT(int16_t, 8, __m128i, _mm_cmpeq_epi32);
X_HI_SET_ONE_INT(int16_t, 8, __m128i, _mm_cmpeq_epi32, _mm_srli_epi16);

X_HI_SET_ALL_ONES_INT(int8_t, 16, __m128i, _mm_cmpeq_epi32);
X_HI_SET_ONE_INT8(int8_t, 16, __m128i, _mm_cmpeq_epi32, _mm_abs_epi8);

X_HI_SET_ALL_ONES_INT(uint64_t, 2, __m128i, _mm_cmpeq_epi32);
X_HI_SET_ONE_INT(uint64_t, 2, __m128i, _mm_cmpeq_epi32, _mm_srli_epi64);

X_HI_SET_ALL_ONES_INT(uint32_t, 4, __m128i, _mm_cmpeq_epi32);
X_HI_SET_ONE_INT(uint32_t, 4, __m128i, _mm_cmpeq_epi32, _mm_srli_epi32);

X_HI_SET_ALL_ONES_INT(uint16_t, 8, __m128i, _mm_cmpeq_epi32);
X_HI_SET_ONE_INT(uint16_t, 8, __m128i, _mm_cmpeq_epi32, _mm_srli_epi16);

X_HI_SET_ALL_ONES_INT(uint8_t, 16, __m128i, _mm_cmpeq_epi32);
X_HI_SET_ONE_INT8(uint8_t, 16, __m128i, _mm_cmpeq_epi32, _mm_abs_epi8);
#endif
#if defined(HI_HAS_AVX2)
X_HI_SET_ALL_ONES_FLOAT(double, 2, __m256d, _mm256_cmpeq_epi32, _mm256_castpd_si256, _mm256_castsi256_pd);
X_HI_SET_ALL_ONES_FLOAT(float, 4, __m256, _mm256_cmpeq_epi32, _mm256_castps_si256, _mm256_castsi256_ps);
X_HI_SET_ALL_ONES_INT(int64_t, 2, __m256i, _mm256_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(int32_t, 4, __m256i, _mm256_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(int16_t, 8, __m256i, _mm256_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(int8_t, 16, __m256i, _mm256_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(uint64_t, 2, __m256i, _mm256_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(uint32_t, 4, __m256i, _mm256_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(uint16_t, 8, __m256i, _mm256_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(uint8_t, 16, __m256i, _mm256_cmpeq_epi32);
#endif
#if defined(HI_HAS_AVX512F)
X_HI_SET_ALL_ONES_FLOAT(double, 2, __m512d, _mm512_cmpeq_epi32, _mm512_castpd_si512, _mm512_castsi512_pd);
X_HI_SET_ALL_ONES_FLOAT(float, 4, __m512, _mm512_cmpeq_epi32, _mm512_castps_si512, _mm512_castsi512_ps);
X_HI_SET_ALL_ONES_INT(int64_t, 2, __m512i, _mm512_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(int32_t, 4, __m512i, _mm512_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(int16_t, 8, __m512i, _mm512_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(int8_t, 16, __m512i, _mm512_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(uint64_t, 2, __m512i, _mm512_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(uint32_t, 4, __m512i, _mm512_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(uint16_t, 8, __m512i, _mm512_cmpeq_epi32);
X_HI_SET_ALL_ONES_INT(uint8_t, 16, __m512i, _mm512_cmpeq_epi32);
#endif

#undef X_HI_SET_ALL_ONES

}} // namespace hi::v1
