

#pragma once

#include "load.hpp"
#include "store.hpp"

hi_export_module(hikogui.simd.binary_operators);

namespace hi { inline namespace v1 {

// clang-format off
template<typename T, size_t N> struct simd_add;
template<typename T, size_t N> struct simd_sub;
template<typename T, size_t N> struct simd_mul;
template<typename T, size_t N> struct simd_div;
template<typename T, size_t N> struct simd_or;
template<typename T, size_t N> struct simd_and;
template<typename T, size_t N> struct simd_andnot;
template<typename T, size_t N> struct simd_max;
template<typename T, size_t N> struct simd_min;
template<typename T, size_t N> struct simd_lt;
template<typename T, size_t N> struct simd_gt;
template<typename T, size_t N> struct simd_le;
template<typename T, size_t N> struct simd_ge;
template<typename T, size_t N> struct simd_eq;
template<typename T, size_t N> struct simd_ne;
// clang-format on

#define HI_X(NAME, VALUE_TYPE, SIZE, FUNC) \
    template<> \
    struct NAME<VALUE_TYPE, SIZE> { \
        using array_type = std::array<VALUE_TYPE, SIZE> \
        [[nodiscard]] hi_inline array_type operator()(array_type const &lhs, array_type const &rhs) const noexcept \
        { \
            hilet lhs_ = simd_load<VALUE_TYPE, SIZE>{}(lhs); \
            hilet rhs_ = simd_load<VALUE_TYPE, SIZE>{}(rhs); \
            return simd_store<VALUE_TYPE, SIZE>{}(lhs_, rhs_); \
        } \
    }

#if defined(HI_HAS_SSE)
HI_X(simd_add, float, 4, _mm_add_ps);
HI_X(simd_sub, float, 4, _mm_sub_ps);
HI_X(simd_mul, float, 4, _mm_mul_ps);
HI_X(simd_div, float, 4, _mm_div_ps);
HI_X(simd_or, float, 4, _mm_or_ps);
HI_X(simd_and, float, 4, _mm_and_ps);
HI_X(simd_andnot, float, 4, _mm_andnot_ps);
HI_X(simd_max, float, 4, _mm_max_ps);
HI_X(simd_min, float, 4, _mm_min_ps);
HI_X(simd_lt, float, 4, _mm_cmplt_ps);
HI_X(simd_gt, float, 4, _mm_cmpgt_ps);
HI_X(simd_le, float, 4, _mm_cmplt_ps);
HI_X(simd_ge, float, 4, _mm_cmpgt_ps);
HI_X(simd_eq, float, 4, _mm_cmpeq_ps);
HI_X(simd_ne, float, 4, _mm_cmpneq_ps);
#endif
#if defined(HI_HAS_SSE2)
HI_X(simd_add, double, 2, _mm_add_pd);
HI_X(simd_sub, double, 2, _mm_sub_pd);
HI_X(simd_mul, double, 2, _mm_mul_pd);
HI_X(simd_div, double, 2, _mm_div_pd);
HI_X(simd_or, double, 2, _mm_or_pd);
HI_X(simd_and, double, 2, _mm_and_pd);
HI_X(simd_andnot, double, 2, _mm_andnot_pd);
HI_X(simd_max, double, 2, _mm_max_pd);
HI_X(simd_min, double, 2, _mm_min_pd);
HI_X(simd_lt, double, 2, _mm_lt_pd);
HI_X(simd_gt, double, 2, _mm_gt_pd);
HI_X(simd_le, double, 2, _mm_le_pd);
HI_X(simd_ge, double, 2, _mm_ge_pd);
HI_X(simd_eq, double, 2, _mm_eq_pd);
HI_X(simd_ne, double, 2, _mm_ne_pd);

HI_X(simd_add, int64_t, 2, _mm_add_epi64);
HI_X(simd_sub, int64_t, 2, _mm_sub_epi64);
HI_X(simd_or, int64_t, 2, _mm_or_si128);
HI_X(simd_and, int64_t, 2, _mm_and_si128);
HI_X(simd_andnot, int64_t, 2, _mm_and_si128);
HI_X(simd_eq, int64_t, 2, _mm_cmpeq_epi64);
HI_X(simd_gt, int64_t, 2, _mm_cmpgt_epi64);

HI_X(simd_add, int32_t, 4, _mm_add_epi32);
HI_X(simd_sub, int32_t, 4, _mm_sub_epi32);
HI_X(simd_mul, int32_t, 4, _mm_mulhrs_epi32);
HI_X(simd_or, int32_t, 4, _mm_or_si128);
HI_X(simd_and, int32_t, 4, _mm_and_si128);
HI_X(simd_andnot, int32_t, 4, _mm_andnot_si128);
HI_X(simd_max, int32_t, 4, _mm_max_epi32);
HI_X(simd_min, int32_t, 4, _mm_min_epi32);
HI_X(simd_lt, int32_t, 4, _mm_lt_epi32);
HI_X(simd_gt, int32_t, 4, _mm_gt_epi32);
HI_X(simd_le, int32_t, 4, _mm_le_epi32);
HI_X(simd_ge, int32_t, 4, _mm_ge_epi32);
HI_X(simd_eq, int32_t, 4, _mm_eq_epi32);
HI_X(simd_ne, int32_t, 4, _mm_ne_epi32);

HI_X(simd_add, int16_t, 8, _mm_add_epi16);
HI_X(simd_sub, int16_t, 8, _mm_sub_epi16);
HI_X(simd_mul, int16_t, 8, _mm_mulhrs_epi16);
HI_X(simd_or, int16_t, 8, _mm_or_si128);
HI_X(simd_and, int16_t, 8, _mm_and_si128);
HI_X(simd_andnot, int16_t, 8, _mm_andnot_si128);
HI_X(simd_max, int16_t, 8, _mm_max_epi32);
HI_X(simd_min, int16_t, 8, _mm_min_epi32);
HI_X(simd_lt, int16_t, 8, _mm_lt_epi16);
HI_X(simd_gt, int16_t, 8, _mm_gt_epi16);
HI_X(simd_le, int16_t, 8, _mm_le_epi16);
HI_X(simd_ge, int16_t, 8, _mm_ge_epi16);
HI_X(simd_eq, int16_t, 8, _mm_eq_epi16);
HI_X(simd_ne, int16_t, 8, _mm_ne_epi16);

HI_X(simd_add, int8_t, 16, _mm_add_epi8);
HI_X(simd_sub, int8_t, 16, _mm_sub_epi8);
HI_X(simd_mul, int8_t, 16, _mm_mulhrs_epi8);
HI_X(simd_or, int8_t, 16, _mm_or_si128);
HI_X(simd_and, int8_t, 16, _mm_and_si128);
HI_X(simd_andnot, int8_t, 16, _mm_andnot_si128);
HI_X(simd_max, int8_t, 16, _mm_max_epi8);
HI_X(simd_min, int8_t, 16, _mm_min_epi8);
HI_X(simd_lt, int8_t, 16, _mm_lt_epi8);
HI_X(simd_gt, int8_t, 16, _mm_gt_epi8);
HI_X(simd_le, int8_t, 16, _mm_le_epi8);
HI_X(simd_ge, int8_t, 16, _mm_ge_epi8);
HI_X(simd_eq, int8_t, 16, _mm_eq_epi8);
HI_X(simd_ne, int16_t, 16, _mm_ne_epi16);

HI_X(simd_add, uint32_t, 4, _mm_add_epi32);
HI_X(simd_sub, uint32_t, 4, _mm_sub_epi32);
HI_X(simd_mul, uint32_t, 4, _mm_mul_epu32);
HI_X(simd_or, uint32_t, 4, _mm_or_si128);
HI_X(simd_and, uint32_t, 4, _mm_and_si128);
HI_X(simd_andnot, uint32_t, 4, _mm_andnot_si128);
HI_X(simd_max, uint32_t, 4, _mm_max_epu32);
HI_X(simd_min, uint32_t, 4, _mm_min_epu32);

HI_X(simd_add, uint16_t, 8, _mm_add_epi16);
HI_X(simd_sub, uint16_t, 8, _mm_sub_epi16);
HI_X(simd_mul, uint16_t, 8, _mm_mul_epu16);
HI_X(simd_or, uint16_t, 8, _mm_or_si128);
HI_X(simd_and, uint16_t, 8, _mm_and_si128);
HI_X(simd_andnot, uint16_t, 8, _mm_andnot_si128);
HI_X(simd_max, uint16_t, 8, _mm_max_epu16);
HI_X(simd_min, uint16_t, 8, _mm_min_epu16);

HI_X(simd_add, uint8_t, 16, _mm_add_epi8);
HI_X(simd_sub, uint8_t, 16, _mm_sub_epi8);
HI_X(simd_mul, uint8_t, 16, _mm_mul_epu8);
HI_X(simd_or, uint8_t, 16, _mm_or_si128);
HI_X(simd_and, uint8_t, 16, _mm_and_si128);
HI_X(simd_andnot, uint8_t, 16, _mm_andnot_si128);
HI_X(simd_max, uint8_t, 16, _mm_max_epu8);
HI_X(simd_min, uint8_t, 16, _mm_min_epu8);
#endif

#if defined(HI_HAS_AVX)
HI_X(simd_add, double, 4, _mm256_add_pd);
HI_X(simd_sub, double, 4, _mm256_sub_pd);
HI_X(simd_mul, double, 4, _mm256_mul_pd);
HI_X(simd_div, double, 4, _mm256_div_pd);
HI_X(simd_or, double, 4, _mm256_or_si256);
HI_X(simd_and, double, 4, _mm256_and_si256);
HI_X(simd_andnot, double, 4, _mm256_andnot_si256);
HI_X(simd_max, double, 4, _mm256_max_pd);
HI_X(simd_min, double, 4, _mm256_min_pd);
HI_X(simd_lt, double, 4, _mm256_lt_pd);
HI_X(simd_gt, double, 4, _mm256_gt_pd);
HI_X(simd_le, double, 4, _mm256_le_pd);
HI_X(simd_ge, double, 4, _mm256_ge_pd);
HI_X(simd_eq, double, 4, _mm256_eq_pd);
HI_X(simd_ne, double, 4, _mm256_ne_pd);

HI_X(simd_add, float, 8, _mm256_add_ps);
HI_X(simd_sub, float, 8, _mm256_sub_ps);
HI_X(simd_mul, float, 8, _mm256_mul_ps);
HI_X(simd_div, float, 8, _mm256_div_ps);
HI_X(simd_or, float, 8, _mm256_or_si256);
HI_X(simd_and, float, 8, _mm256_and_si256);
HI_X(simd_andnot, float, 8, _mm256_andnot_si256);
HI_X(simd_max, float, 8, _mm256_max_ps);
HI_X(simd_min, float, 8, _mm256_min_ps);
HI_X(simd_lt, float, 8, _mm256_lt_ps);
HI_X(simd_gt, float, 8, _mm256_gt_ps);
HI_X(simd_le, float, 8, _mm256_le_ps);
HI_X(simd_ge, float, 8, _mm256_ge_ps);
HI_X(simd_eq, float, 8, _mm256_eq_ps);
HI_X(simd_ne, float, 8, _mm256_ne_ps);
#endif

#if defined(HI_HAS_AVX2)
HI_X(simd_add, int64_t, 4, _mm256_add_epi64);
HI_X(simd_sub, int64_t, 4, _mm256_sub_epi64);
HI_X(simd_mul, int64_t, 4, _mm256_mul_epi64);
HI_X(simd_div, int64_t, 4, _mm256_div_epi64);
HI_X(simd_or, int64_t, 4, _mm256_or_si256);
HI_X(simd_and, int64_t, 4, _mm256_and_si256);
HI_X(simd_andnot, int64_t, 4, _mm256_andnot_si256);
HI_X(simd_max, int64_t, 4, _mm256_max_epi32);
HI_X(simd_min, int64_t, 4, _mm256_min_epi32);
HI_X(simd_lt, int64_t, 4, _mm256_lt_epi64);
HI_X(simd_gt, int64_t, 4, _mm256_gt_epi64);
HI_X(simd_le, int64_t, 4, _mm256_le_epi64);
HI_X(simd_ge, int64_t, 4, _mm256_ge_epi64);
HI_X(simd_eq, int64_t, 4, _mm256_eq_epi64);
HI_X(simd_ne, int64_t, 4, _mm256_ne_epi64);

HI_X(simd_add, int32_t, 8, _mm256_add_epi32);
HI_X(simd_sub, int32_t, 8, _mm256_sub_epi32);
HI_X(simd_mul, int32_t, 8, _mm256_mul_epi32);
HI_X(simd_div, int32_t, 8, _mm256_div_epi32);
HI_X(simd_or, int32_t, 8, _mm256_or_si256);
HI_X(simd_and, int32_t, 8, _mm256_and_si256);
HI_X(simd_andnot, int32_t, 8, _mm256_andnot_si256);
HI_X(simd_max, int32_t, 8, _mm256_max_epi32);
HI_X(simd_min, int32_t, 8, _mm256_min_epi32);
HI_X(simd_lt, int32_t, 8, _mm256_lt_epi32);
HI_X(simd_gt, int32_t, 8, _mm256_gt_epi32);
HI_X(simd_le, int32_t, 8, _mm256_le_epi32);
HI_X(simd_ge, int32_t, 8, _mm256_ge_epi32);
HI_X(simd_eq, int32_t, 8, _mm256_eq_epi32);
HI_X(simd_ne, int32_t, 8, _mm256_ne_epi32);

HI_X(simd_add, int16_t, 16, _mm256_add_epi16);
HI_X(simd_sub, int16_t, 16, _mm256_sub_epi16);
HI_X(simd_mul, int16_t, 16, _mm256_mul_epi16);
HI_X(simd_div, int16_t, 16, _mm256_div_epi16);
HI_X(simd_or, int16_t, 16, _mm256_or_si256);
HI_X(simd_and, int16_t, 16, _mm256_and_si256);
HI_X(simd_andnot, int16_t, 16, _mm256_andnot_si256);
HI_X(simd_max, int16_t, 16, _mm256_max_ps);
HI_X(simd_min, int16_t, 16, _mm256_min_ps);
HI_X(simd_lt, int16_t, 16, _mm256_lt_epi16);
HI_X(simd_gt, int16_t, 16, _mm256_gt_epi16);
HI_X(simd_le, int16_t, 16, _mm256_le_epi16);
HI_X(simd_ge, int16_t, 16, _mm256_ge_epi16);
HI_X(simd_eq, int16_t, 16, _mm256_eq_epi16);
HI_X(simd_ne, int16_t, 16, _mm256_ne_epi16);

HI_X(simd_add, int8_t, 32, _mm256_add_epi8);
HI_X(simd_sub, int8_t, 32, _mm256_sub_epi8);
HI_X(simd_mul, int8_t, 32, _mm256_mul_epi8);
HI_X(simd_div, int8_t, 32, _mm256_div_epi8);
HI_X(simd_or, int8_t, 32, _mm256_or_si256);
HI_X(simd_and, int8_t, 32, _mm256_and_si256);
HI_X(simd_andnot, int8_t, 32, _mm256_andnot_si256);
HI_X(simd_max, int8_t, 32, _mm256_max_epi8);
HI_X(simd_min, int8_t, 32, _mm256_min_epi8);
HI_X(simd_lt, int8_t, 32, _mm256_lt_epi8);
HI_X(simd_gt, int8_t, 32, _mm256_gt_epi8);
HI_X(simd_le, int8_t, 32, _mm256_le_epi8);
HI_X(simd_ge, int8_t, 32, _mm256_ge_epi8);
HI_X(simd_eq, int8_t, 32, _mm256_eq_epi8);
HI_X(simd_ne, int8_t, 32, _mm256_ne_epi8);

HI_X(simd_add, uint64_t, 4, _mm256_add_epu64);
HI_X(simd_sub, uint64_t, 4, _mm256_sub_epu64);
HI_X(simd_mul, uint64_t, 4, _mm256_mul_epu64);
HI_X(simd_div, uint64_t, 4, _mm256_div_epu64);
HI_X(simd_or, uint64_t, 4, _mm256_or_si256);
HI_X(simd_and, uint64_t, 4, _mm256_and_si256);
HI_X(simd_andnot, uint64_t, 4, _mm256_andnot_si256);
HI_X(simd_max, uint64_t, 4, _mm256_max_epu64);
HI_X(simd_min, uint64_t, 4, _mm256_min_epu64);
HI_X(simd_lt, uint64_t, 4, _mm256_lt_epu64);
HI_X(simd_gt, uint64_t, 4, _mm256_gt_epu64);
HI_X(simd_le, uint64_t, 4, _mm256_le_epu64);
HI_X(simd_ge, uint64_t, 4, _mm256_ge_epu64);
HI_X(simd_eq, uint64_t, 4, _mm256_eq_epu64);
HI_X(simd_ne, uint64_t, 4, _mm256_ne_epu64);

HI_X(simd_add, uint32_t, 8, _mm256_add_epu32);
HI_X(simd_sub, uint32_t, 8, _mm256_sub_epu32);
HI_X(simd_mul, uint32_t, 8, _mm256_mul_epu32);
HI_X(simd_div, uint32_t, 8, _mm256_div_epu32);
HI_X(simd_or, uint32_t, 8, _mm256_or_si256);
HI_X(simd_and, uint32_t, 8, _mm256_and_si256);
HI_X(simd_andnot, uint32_t, 8, _mm256_andnot_si256);
HI_X(simd_max, uint32_t, 8, _mm256_max_epu32);
HI_X(simd_min, uint32_t, 8, _mm256_min_epu32);
HI_X(simd_lt, uint32_t, 8, _mm256_lt_epu32);
HI_X(simd_gt, uint32_t, 8, _mm256_gt_epu32);
HI_X(simd_le, uint32_t, 8, _mm256_le_epu32);
HI_X(simd_ge, uint32_t, 8, _mm256_ge_epu32);
HI_X(simd_eq, uint32_t, 8, _mm256_eq_epu32);
HI_X(simd_ne, uint32_t, 8, _mm256_ne_epu32);

HI_X(simd_add, uint16_t, 16, _mm256_add_epi16);
HI_X(simd_sub, uint16_t, 16, _mm256_sub_epi16);
HI_X(simd_mul, uint16_t, 16, _mm256_mul_epu16);
HI_X(simd_div, uint16_t, 16, _mm256_div_epu16);
HI_X(simd_or, uint16_t, 16, _mm256_or_si256);
HI_X(simd_and, uint16_t, 16, _mm256_and_si256);
HI_X(simd_andnot, uint16_t, 16, _mm256_andnot_si256);
HI_X(simd_max, uint16_t, 16, _mm256_max_epu16);
HI_X(simd_min, uint16_t, 16, _mm256_min_epu16);
HI_X(simd_lt, uint16_t, 16, _mm256_lt_epu16);
HI_X(simd_gt, uint16_t, 16, _mm256_gt_epu16);
HI_X(simd_le, uint16_t, 16, _mm256_le_epu16);
HI_X(simd_ge, uint16_t, 16, _mm256_ge_epu16);
HI_X(simd_eq, uint16_t, 16, _mm256_eq_epu16);
HI_X(simd_ne, uint16_t, 16, _mm256_ne_epu16);

HI_X(simd_add, uint8_t, 32, _mm256_add_epi8);
HI_X(simd_sub, uint8_t, 32, _mm256_sub_epi8);
HI_X(simd_mul, uint8_t, 32, _mm256_mul_epu8);
HI_X(simd_div, uint8_t, 32, _mm256_div_epu8);
HI_X(simd_or, uint8_t, 32, _mm256_or_si256);
HI_X(simd_and, uint8_t, 32, _mm256_and_si256);
HI_X(simd_andnot, uint8_t, 32, _mm256_andnot_si256);
HI_X(simd_max, uint8_t, 32, _mm256_max_epu8);
HI_X(simd_min, uint8_t, 32, _mm256_min_epu8);
HI_X(simd_lt, uint8_t, 32, _mm256_lt_epu8);
HI_X(simd_gt, uint8_t, 32, _mm256_gt_epu8);
HI_X(simd_le, uint8_t, 32, _mm256_le_epu8);
HI_X(simd_ge, uint8_t, 32, _mm256_ge_epu8);
HI_X(simd_eq, uint8_t, 32, _mm256_eq_epu8);
HI_X(simd_ne, uint8_t, 32, _mm256_ne_epu8);

#endif

#if defined(HI_HAS_AVX512F)
HI_X(float, 16, _mm512_add_ps);
HI_X(double, 8, _mm512_add_pd);
HI_X(int64_t, 8, _mm512_add_epi64);
HI_X(int32_t, 16, _mm512_add_epi32);
HI_X(int16_t, 32, _mm512_add_epi16);
HI_X(int8_t, 64, _mm512_add_epi8);
#endif

#undef HI_X

}}

