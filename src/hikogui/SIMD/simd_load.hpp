// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <array>

#pragma once

hi_export_module(hikogui.simd.load);

namespace hi { inline namespace v1 {

template<typename T, size_t N>
struct simd_load;

#define HI_X(VALUE_TYPE, SIZE, REG, FUNC) \
    template<> \
    struct simd_load<VALUE_TYPE, SIZE> { \
        using array_type = std::array<VALUE_TYPE, SIZE> \
        [[nodiscard]] hi_inline REG operator()(array_type const &rhs) const noexcept \
        { \
            return FUNC(rhs.data()); \
        } \
    }

#if defined(HI_HAS_SSE)
HI_X(float, 4, __m128, _mm_loadu_ps);
#endif
#if defined(HI_HAS_SSE2)
HI_X(double, 2, __m128d, _mm_loadu_pd);
HI_X(uint64_t, 2, __m128i, _mm_loadu_si128);
HI_X(uint32_t, 4, __m128i, _mm_loadu_si128);
HI_X(uint16_t, 8, __m128i, _mm_loadu_si128);
HI_X(uint8_t, 16, __m128i, _mm_loadu_si128);
HI_X(int64_t, 2, __m128i, _mm_loadu_si128);
HI_X(int32_t, 4, __m128i, _mm_loadu_si128);
HI_X(int16_t, 8, __m128i, _mm_loadu_si128);
HI_X(int8_t, 16, __m128i, _mm_loadu_si128);
#endif
#if defined(HI_HAS_AVX)
HI_X(float, 8, __m256, _mm256_loadu_ps);
HI_X(double, 4, __m256d, _mm256_loadu_pd);
HI_X(uint64_t, 4, __m256i, _mm256_loadu_si256);
HI_X(uint32_t, 8, __m256i, _mm256_loadu_si256);
HI_X(uint16_t, 16, __m256i, _mm256_loadu_si256);
HI_X(uint8_t, 32, __m256i, _mm256_loadu_si256);
HI_X(int64_t, 4, __m256i, _mm256_loadu_si256);
HI_X(int32_t, 8, __m256i, _mm256_loadu_si256);
HI_X(int16_t, 16, __m256i, _mm256_loadu_si256);
HI_X(int8_t, 32, __m256i, _mm256_loadu_si256);
#endif
#if defined(HI_HAS_AVX512F)
HI_X(float, 16, __m512, _mm512_loadu_ps);
HI_X(double, 8, __m512d, _mm512_loadu_pd);
HI_X(uint64_t, 8, __m512i, _mm512_loadu_si512);
HI_X(uint32_t, 16, __m512i, _mm512_loadu_si512);
HI_X(uint16_t, 32, __m512i, _mm512_loadu_si512);
HI_X(uint8_t, 64, __m512i, _mm512_loadu_si512);
HI_X(int64_t, 8, __m512i, _mm512_loadu_si512);
HI_X(int32_t, 16, __m512i, _mm512_loadu_si512);
HI_X(int16_t, 32, __m512i, _mm512_loadu_si512);
HI_X(int8_t, 64, __m512i, _mm512_loadu_si512);
#endif

#undef HI_X

}}

