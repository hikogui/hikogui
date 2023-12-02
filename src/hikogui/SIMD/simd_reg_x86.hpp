
#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.simd.simd_reg);

hi_export namespace hi { inline namespace v1 {

// clang-format off
template<typename T, size_t N> struct simd_reg;

#if defined(HI_HAS_SSE)
template<> struct simd_reg<float, 4> { using type = __m128; };
#endif
#if defined(HI_HAS_SSE2)
template<> struct simd_reg<double, 2> { using type = __m128d; };
template<> struct simd_reg<half, 8> { using type = __m128i; };
template<> struct simd_reg<int64_t, 2> { using type = __m128i; };
template<> struct simd_reg<int32_t, 4> { using type = __m128i; };
template<> struct simd_reg<int16_t, 8> { using type = __m128i; };
template<> struct simd_reg<int8_t, 16> { using type = __m128i; };
template<> struct simd_reg<uint64_t, 2> { using type = __m128i; };
template<> struct simd_reg<uint32_t, 4> { using type = __m128i; };
template<> struct simd_reg<uint16_t, 8> { using type = __m128i; };
template<> struct simd_reg<uint8_t, 16> { using type = __m128i; };
#endif
#if defined(HI_HAS_AVX)
template<> struct simd_reg<double, 4> { using type = __m256d; };
template<> struct simd_reg<float, 8> { using type = __m256; };
#endif
#if defined(HI_HAS_AVX2)
template<> struct simd_reg<half, 16> { using type = __m256i; };
template<> struct simd_reg<int64_t, 4> { using type = __m256i; };
template<> struct simd_reg<int32_t, 8> { using type = __m256i; };
template<> struct simd_reg<int16_t, 16> { using type = __m256i; };
template<> struct simd_reg<int8_t, 32> { using type = __m256i; };
template<> struct simd_reg<uint64_t, 4> { using type = __m256i; };
template<> struct simd_reg<uint32_t, 8> { using type = __m256i; };
template<> struct simd_reg<uint16_t, 16> { using type = __m256i; };
template<> struct simd_reg<uint8_t, 32> { using type = __m256i; };
#endif
#if defined(HI_HAS_AVX512F)
template<> struct simd_reg<double, 8> { using type = __m512d; };
template<> struct simd_reg<float, 16> { using type = __m512; };
template<> struct simd_reg<half, 32> { using type = __m512i; };
template<> struct simd_reg<int64_t, 8> { using type = __m512i; };
template<> struct simd_reg<int32_t, 16> { using type = __m512i; };
template<> struct simd_reg<int16_t, 32> { using type = __m512i; };
template<> struct simd_reg<int8_t, 64> { using type = __m512i; };
template<> struct simd_reg<uint64_t, 8> { using type = __m512i; };
template<> struct simd_reg<uint32_t, 16> { using type = __m512i; };
template<> struct simd_reg<uint16_t, 32> { using type = __m512i; };
template<> struct simd_reg<uint8_t, 64> { using type = __m512i; };
#endif

// clang-format on

template<typename T, size_t N>
using simd_reg_t = simd_reg<T, N>::type;

}}
