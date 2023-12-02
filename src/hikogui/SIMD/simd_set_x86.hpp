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
template<typename T, size_t N> struct simd_set_zero;
// clang-format on

#define HI_X(NAME, TYPE, SIZE, REG, OP) \
    template<> \
    struct NAME<TYPE, SIZE> { \
        [[nodiscard]] hi_force_inline REG operator()() const noexcept \
        { \
            return OP; \
        } \
    }

#if defined(HI_HAS_SSE)
HI_X(simd_set_zero, float, 4, __m128, _mm_setzero_ps());
#endif
#if defined(HI_HAS_SSE2)
HI_X(simd_set_zero, double, 2, __m128d, _mm_setzero_pd());
HI_X(simd_set_zero, int64_t, 2, __m128i, _mm_setzero_si128());
HI_X(simd_set_zero, int32_t, 4, __m128i, _mm_setzero_si128());
HI_X(simd_set_zero, int16_t, 8, __m128i, _mm_setzero_si128());
HI_X(simd_set_zero, int8_t, 16, __m128i, _mm_setzero_si128());
HI_X(simd_set_zero, uint64_t, 2, __m128i, _mm_setzero_si128());
HI_X(simd_set_zero, uint32_t, 4, __m128i, _mm_setzero_si128());
HI_X(simd_set_zero, uint16_t, 8, __m128i, _mm_setzero_si128());
HI_X(simd_set_zero, uint8_t, 16, __m128i, _mm_setzero_si128());
#endif
#if defined(HI_HAS_AVX)
HI_X(simd_set_zero, double, 4, __m256d, _mm256_setzero_pd());
HI_X(simd_set_zero, float, 8, __m256, _mm256_setzero_ps());
#endif
#if defined(HI_HAS_AVX2)
HI_X(simd_set_zero, int64_t, 4, __m256i, _mm256_setzero_si256());
HI_X(simd_set_zero, int32_t, 8, __m256i, _mm256_setzero_si256());
HI_X(simd_set_zero, int16_t, 16, __m256i, _mm256_setzero_si256());
HI_X(simd_set_zero, int8_t, 32, __m256i, _mm256_setzero_si256());
HI_X(simd_set_zero, uint64_t, 4, __m256i, _mm256_setzero_si256());
HI_X(simd_set_zero, uint32_t, 8, __m256i, _mm256_setzero_si256());
HI_X(simd_set_zero, uint16_t, 16, __m256i, _mm256_setzero_si256());
HI_X(simd_set_zero, uint8_t, 32, __m256i, _mm256_setzero_si256());
#endif
#if defined(HI_HAS_AVX512F)
HI_X(simd_set_zero, double, 8, __m512d, _mm512_setzero_pd());
HI_X(simd_set_zero, float, 16, __m512, _mm512_setzero_ps());
HI_X(simd_set_zero, int64_t, 8, __m512i, _mm512_setzero_si512());
HI_X(simd_set_zero, int32_t, 16, __m512i, _mm512_setzero_si512());
HI_X(simd_set_zero, int16_t, 32, __m512i, _mm512_setzero_si512());
HI_X(simd_set_zero, int8_t, 64, __m512i, _mm512_setzero_si512());
HI_X(simd_set_zero, uint64_t, 8, __m512i, _mm512_setzero_si512());
HI_X(simd_set_zero, uint32_t, 16, __m512i, _mm512_setzero_si512());
HI_X(simd_set_zero, uint16_t, 32, __m512i, _mm512_setzero_si512());
HI_X(simd_set_zero, uint8_t, 64, __m512i, _mm512_setzero_si512());
#endif

#if defined(HI_HAS_SSE2)
HI_X(simd_set_all_ones, float, 4, __m128, _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128())));
#elif defined(HI_HAS_SSE)
HI_X(simd_set_all_ones, float, 4, __m128, _mm_cmpeq_pd(_mm_setzero_ps(), _mm_setzero_ps()));
#endif
#if defined(HI_HAS_SSE2)
HI_X(simd_set_all_ones, double, 2, __m128d, _mm_castsi128_pd(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128())));
HI_X(simd_set_all_ones, int64_t, 2, __m128i, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
HI_X(simd_set_all_ones, int32_t, 4, __m128i, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
HI_X(simd_set_all_ones, int16_t, 8, __m128i, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
HI_X(simd_set_all_ones, int8_t, 16, __m128i, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
HI_X(simd_set_all_ones, uint64_t, 2, __m128i, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
HI_X(simd_set_all_ones, uint32_t, 4, __m128i, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
HI_X(simd_set_all_ones, uint16_t, 8, __m128i, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
HI_X(simd_set_all_ones, uint8_t, 16, __m128i, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
#endif
#if defined(HI_HAS_AVX2)
HI_X(simd_set_all_ones, double, 4, __m256d, _mm256_castsi256_pd(_mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256())));
HI_X(simd_set_all_ones, float, 8, __m256, _mm256_castsi256_ps(_mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256()));
#elif defined(HI_HAS_AVX)
HI_X(simd_set_all_ones, double, 4, __m256d, _mm256_cmp_pd(_mm256_setzero_pd(), _mm256_setzero_pd(), _CMP_EQ_OQ));
HI_X(simd_set_all_ones, float, 8, __m256, _mm256_cmp_ps(_mm256_setzero_ps(), _mm256_setzero_ps(), _CMP_EQ_OQ));
#endif
#if defined(HI_HAS_AVX2)
HI_X(simd_set_all_ones, int64_t, 4, __m256i, _mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256()));
HI_X(simd_set_all_ones, int32_t, 8, __m256i, _mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256()));
HI_X(simd_set_all_ones, int16_t, 16, __m256i, _mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256()));
HI_X(simd_set_all_ones, int8_t, 32, __m256i, _mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256()));
HI_X(simd_set_all_ones, uint64_t, 4, __m256i, _mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256()));
HI_X(simd_set_all_ones, uint32_t, 8, __m256i, _mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256()));
HI_X(simd_set_all_ones, uint16_t, 16, __m256i, _mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256()));
HI_X(simd_set_all_ones, uint8_t, 32, __m256i, _mm256_cmpeq_epi32(_mm256_setzero_si256(), _mm256_setzero_si256()));
#endif
#if defined(HI_HAS_AVX512F)
HI_X(simd_set_all_ones, double, 8, __m512d, _mm512_castsi512_pd(_mm512_cmpeq_epi32_mask(_mm512_setzero_si512(), _mm512_setzero_si512())));
HI_X(simd_set_all_ones, float, 16, __m512, _mm512_castsi512_ps(_mm512_cmpeq_epi32_mask(_mm512_setzero_si512(), _mm512_setzero_si512())));
HI_X(simd_set_all_ones, int64_t, 8, __m512i, _mm512_cmpeq_epi32_mask(_mm512_setzero_si512(), _mm512_setzero_si512()));
HI_X(simd_set_all_ones, int32_t, 16, __m512i, _mm512_cmpeq_epi32_mask(_mm512_setzero_si512(), _mm512_setzero_si512()));
HI_X(simd_set_all_ones, int16_t, 32, __m512i, _mm512_cmpeq_epi32_mask(_mm512_setzero_si512(), _mm512_setzero_si512()));
HI_X(simd_set_all_ones, int8_t, 64, __m512i, _mm512_cmpeq_epi32_mask(_mm512_setzero_si512(), _mm512_setzero_si512()));
HI_X(simd_set_all_ones, uint64_t, 8, __m512i, _mm512_cmpeq_epi32_mask(_mm512_setzero_si512(), _mm512_setzero_si512()));
HI_X(simd_set_all_ones, uint32_t, 16, __m512i, _mm512_cmpeq_epi32_mask(_mm512_setzero_si512(), _mm512_setzero_si512()));
HI_X(simd_set_all_ones, uint16_t, 32, __m512i, _mm512_cmpeq_epi32_mask(_mm512_setzero_si512(), _mm512_setzero_si512()));
HI_X(simd_set_all_ones, uint8_t, 64, __m512i, _mm512_cmpeq_epi32_mask(_mm512_setzero_si512(), _mm512_setzero_si512()));
#endif

#if defined(HI_HAS_SSE2)
X_HI(simd_set_one, float, 4, __m128, _mm_castsi128_ps(_mm_srli(_mm_slli_epi32(simd_set_all_ones<uint32_t, 4>{}(), 25), 2)));
#elif defined(HI_HAS_SSE)
X_HI(simd_set_one, float, 4, __m128, _mm_set1_ps(1.0f));
#endif
#if defined(HI_HAS_SSE2)
X_HI(simd_set_one, double, 2, __m128d, _mm_castsi128_pd(_mm_srli(_mm_slli_epi64(simd_set_all_ones<uint32_t, 4>{}(), 54), 2)));
X_HI(simd_set_one, int64_t, 2, __m128i, _mm_srli_epi64(simd_set_all_ones<uint32_t, 4>{}(), 63));
X_HI(simd_set_one, int32_t, 4, __m128i, _mm_srli_epi32(simd_set_all_ones<uint32_t, 4>{}(), 31));
X_HI(simd_set_one, int16_t, 8, __m128i, _mm_srli_epi16(simd_set_all_ones<uint32_t, 4>{}(), 15));
X_HI(simd_set_one, int8_t, 16, __m128i, _mm_abs_epi8(simd_set_all_ones<uint32_t, 4>{}()));
X_HI(simd_set_one, uint64_t, 2, __m128i, _mm_srli_epi64(simd_set_all_ones<uint32_t, 4>{}(), 63));
X_HI(simd_set_one, uint32_t, 4, __m128i, _mm_srli_epi32(simd_set_all_ones<uint32_t, 4>{}(), 31));
X_HI(simd_set_one, uint16_t, 8, __m128i, _mm_srli_epi16(simd_set_all_ones<uint32_t, 4>{}(), 15));
X_HI(simd_set_one, uint8_t, 16, __m128i, _mm_abs_epi8(simd_set_all_ones<uint32_t, 4>{}()));
#endif
#if defined(HI_HAS_AVX2)
X_HI(simd_set_one, double, 4, __m256d, _mm256_castsi256_pd(_mm256_srli(_mm256_slli_epi64(simd_set_all_ones<uint32_t, 8>{}(), 54), 2)));
X_HI(simd_set_one, float, 8, __m256, _mm256_castsi256_ps(_mm256_srli(_mm256_slli_epi32(simd_set_all_ones<uint32_t, 8>{}(), 25), 2)));
#elif defined(HI_HAS_AVX)
X_HI(simd_set_one, double, 4, __m256d, _mm256_set1_pd(1.0));
X_HI(simd_set_one, float, 8, __m256, _mm256_set1_ps(1.0f));
#endif
#if defined(HI_HAS_AVX2)
X_HI(simd_set_one, int64_t, 4, __m256i, _mm256_srli_epi64(simd_set_all_ones<uint32_t, 8>{}(), 63));
X_HI(simd_set_one, int32_t, 8, __m256i, _mm256_srli_epi32(simd_set_all_ones<uint32_t, 8>{}(), 31));
X_HI(simd_set_one, int16_t, 16, __m256i, _mm256_srli_epi16(simd_set_all_ones<uint32_t, 8>{}(), 15));
X_HI(simd_set_one, int8_t, 32, __m256i, _mm256_abs_epi8(simd_set_all_ones<uint32_t, 8>{}()));
X_HI(simd_set_one, uint64_t, 4, __m256i, _mm256_srli_epi64(simd_set_all_ones<uint32_t, 8>{}(), 63));
X_HI(simd_set_one, uint32_t, 8, __m256i, _mm256_srli_epi32(simd_set_all_ones<uint32_t, 8>{}(), 31));
X_HI(simd_set_one, uint16_t, 16, __m256i, _mm256_srli_epi16(simd_set_all_ones<uint32_t, 8>{}(), 15));
X_HI(simd_set_one, uint8_t, 32, __m256i, _mm256_abs_epi8(simd_set_all_ones<uint32_t, 8>{}()));
#endif
#if defined(HI_HAS_AVX512F)
X_HI(simd_set_one, double, 8, __m512d, _mm512_castsi512_pd(_mm512_srli(_mm512_slli_epi64(simd_set_all_ones<uint32_t, 16>{}(), 54), 2)));
X_HI(simd_set_one, float, 16, __m512, _mm512_castsi512_ps(_mm512_srli(_mm512_slli_epi32(simd_set_all_ones<uint32_t, 16>{}(), 25), 2)));
X_HI(simd_set_one, int64_t, 8, __m512i, _mm512_srli_epi64(simd_set_all_ones<uint32_t, 16>{}(), 63));
X_HI(simd_set_one, int32_t, 16, __m512i, _mm512_srli_epi32(simd_set_all_ones<uint32_t, 16>{}(), 31));
X_HI(simd_set_one, int16_t, 32, __m512i, _mm512_srli_epi16(simd_set_all_ones<uint32_t, 16>{}(), 15));
X_HI(simd_set_one, int8_t, 64, __m512i, _mm512_abs_epi8(simd_set_all_ones<uint32_t, 16>{}()));
X_HI(simd_set_one, uint64_t, 8, __m512i, _mm512_srli_epi64(simd_set_all_ones<uint32_t, 16>{}(), 63));
X_HI(simd_set_one, uint32_t, 16, __m512i, _mm512_srli_epi32(simd_set_all_ones<uint32_t, 16>{}(), 31));
X_HI(simd_set_one, uint16_t, 32, __m512i, _mm512_srli_epi16(simd_set_all_ones<uint32_t, 16>{}(), 15));
X_HI(simd_set_one, uint8_t, 64, __m512i, _mm512_abs_epi8(simd_set_all_ones<uint32_t, 16>{}()));
#endif

#undef HI_X
}} // namespace hi::v1
