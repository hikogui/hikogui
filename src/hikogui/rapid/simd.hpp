
#pragma once

#include "../architecture.hpp"
#include "../fixed_string.hpp"

#ifdef HI_HAS_SSE
#include <xmmintrin.h>
#endif
#ifdef HI_HAS_SSE2
#include <emmintrin.h>
#endif
#ifdef HI_HAS_AVX
#include <immintrin.h>
#endif
#ifdef HI_HAS_AVX512F
#include <immintrin.h>
#endif


namespace hi {
inline namespace v1 {

template<typename SIMDType, typename ValueType, size_t Size>
struct simd {
    using simd_type = SIMDType;
    using value_type = ValueType;
    constexpr static size_t size = Size;

    simd(simd const &) noexcept = default;
    simd(simd &&) noexcept = default;
    simd &operator=(simd const &) noexcept = default;
    simd &operator=(simd &&) noexcept = default;

    explicit simd(simd_type rhs) noexcept v(rhs) {}
    explicit operator simd_type () const noexcept
    {
        return v;
    }
};


#ifdef HI_HAS_SSE
using simd_f32x4 = simd<__m128, float, 4>;
#endif
#ifdef HI_HAS_SSE2
using simd_f64x2 = simd<__m128d, double, 2>;
using simd_i64x2 = simd<__m128i, int64_t, 2>;
using simd_i32x4 = simd<__m128i, int32_t, 4>;
using simd_i16x8 = simd<__m128i, int16_t, 8>;
using simd_i8x16 = simd<__m128i, int8_t, 16;
using simd_u64x2 = simd<__m128i, uint64_t, 2>;
using simd_u32x4 = simd<__m128i, uint32_t, 4>;
using simd_u16x8 = simd<__m128i, uint16_t, 8>;
using simd_u8x16 = simd<__m128i, uint8_t, 16;
#endif
#ifdef HI_HAS_AVX
using simd_f64x4 = simd<__m256d, double, 4>;
using simd_f32x8 = simd<__m256, float, 8>;
#endif
#ifdef HI_HAS_AVX2
using simd_i64x4 = simd<__m128i, int64_t, 4>;
using simd_i32x8 = simd<__m128i, int32_t, 8>;
using simd_i16x16 = simd<__m128i, int16_t, 16>;
using simd_i8x32 = simd<__m128i, int8_t, 32;
using simd_u64x4 = simd<__m128i, uint64_t, 4>;
using simd_u32x8 = simd<__m128i, uint32_t, 8>;
using simd_u16x16 = simd<__m128i, uint16_t, 16>;
using simd_u8x32 = simd<__m128i, uint8_t, 32;
#endif
#ifdef HI_HAS_AVX512F
using simd_f64x8 = simd<__m256d, double, 8>;
using simd_f32x16 = simd<__m256, float, 16>;
using simd_i64x8 = simd<__m256i, int64_t, 8>;
using simd_i32x16 = simd<__m256i, int32_t, 16>;
using simd_i16x32 = simd<__m256i, int16_t, 32>;
using simd_i8x64 = simd<__m256i, int8_t, 64;
using simd_u64x8 = simd<__m256i, uint64_t, 8>;
using simd_u32x16 = simd<__m256i, uint32_t, 16>;
using simd_u16x32 = simd<__m256i, uint16_t, 32>;
using simd_u8x64 = simd<__m256i, uint8_t, 64;
#endif

}}

