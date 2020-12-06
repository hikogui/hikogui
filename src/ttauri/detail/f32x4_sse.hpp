
#pragma once

#include "../aligned_array.hpp"

namespace tt {

inline __m128 to_m128(aligned_array<float,4> value) noexcept
{
    return _mm_load_ps(value.data());
}

inline aligned_array<float,4> to_aligned_array_f32x4(__m128 value) noexcept
{
    aligned_array<float,4> r;
    _mm_store_ps(r.data(), value);
    return r;
}

inline aligned_array<float,4> f32x4_sse_ceil(aligned_array<float,4> const &rhs) noexcept
{
    return to_aligned_array_f32x4(_mm_ceil_ps(to_m128(rhs)));
}

inline aligned_array<float,4> f32x4_sse_floor(aligned_array<float,4> const &rhs) noexcept
{
    return to_aligned_array_f32x4(_mm_floor_ps(to_m128(rhs)));
}

inline aligned_array<float,4> f32x4_sse_round(aligned_array<float,4> const &rhs) noexcept
{
    return to_aligned_array_f32x4(_mm_round_ps(to_m128(rhs), _MM_FROUND_CUR_DIRECTION));
}

inline aligned_array<float,4> f32x4_sse_rcp(aligned_array<float,4> const &rhs) noexcept
{
    return to_aligned_array_f32x4(_mm_rcp_ps(to_m128(rhs)));
}

template<size_t D>
inline float f32x4_sse_dot(aligned_array<float,4> const &lhs, aligned_array<float,4> const &rhs) noexcept
{
    static_assert(D <= 4);
    constexpr int imm8 = 0xff >> (4 - D);

    auto tmp = to_aligned_array_f32x4(_mm_dp_ps(to_m128(lhs), to_m128(rhs), imm8));
    return tmp.v[0];
}

template<size_t D>
inline float f32x4_sse_length(aligned_array<float,4> const &rhs) noexcept
{
    static_assert(D <= 4);
    constexpr int imm8 = 0xff >> (4 - D);

    auto _rhs = to_m128(rhs);
    auto tmp = to_aligned_array_f32x4(_mm_sqrt_ps(_mm_dp_ps(_rhs, _rhs), imm8));
    return tmp.v[0];
}

template<size_t D>
inline float f32x4_sse_rcp_length(aligned_array<float,4> const &rhs) noexcept
{
    static_assert(D <= 4);
    constexpr int imm8 = 0xff >> (4 - D);

    auto _rhs = to_m128(rhs);
    auto tmp = to_aligned_array_f32x4(_mm_rsqrt_ps(_mm_dp_ps(_rhs, _rhs), imm8));
    return tmp.v[0];
}

inline bool f32x4_sse_eq(aligned_array<float,4> const &lhs, aligned_array<float,4> const &rhs) noexcept
{
    return _mm_testz_ps(_mm_cmpneq_ps(to_m128(lhs), to_m128(rhs)));
}

template<char A, char B, char C, char D>
[[nodiscard]] constexpr static int f32x4_sse_permute_mask() noexcept
{
    int r = 0;
    switch (A) {
    case 'x': r |= 0b00'00'00'00; break;
    case 'y': r |= 0b00'00'00'01; break;
    case 'z': r |= 0b00'00'00'10; break;
    case 'w': r |= 0b00'00'00'11; break;
    case '0': r |= 0b00'00'00'00; break;
    case '1': r |= 0b00'00'00'00; break;
    }
    switch (B) {
    case 'x': r |= 0b00'00'00'00; break;
    case 'y': r |= 0b00'00'01'00; break;
    case 'z': r |= 0b00'00'10'00; break;
    case 'w': r |= 0b00'00'11'00; break;
    case '0': r |= 0b00'00'01'00; break;
    case '1': r |= 0b00'00'01'00; break;
    }
    switch (C) {
    case 'x': r |= 0b00'00'00'00; break;
    case 'y': r |= 0b00'01'00'00; break;
    case 'z': r |= 0b00'10'00'00; break;
    case 'w': r |= 0b00'11'00'00; break;
    case '0': r |= 0b00'10'00'00; break;
    case '1': r |= 0b00'10'00'00; break;
    }
    switch (D) {
    case 'x': r |= 0b00'00'00'00; break;
    case 'y': r |= 0b01'00'00'00; break;
    case 'z': r |= 0b10'00'00'00; break;
    case 'w': r |= 0b11'00'00'00; break;
    case '0': r |= 0b11'00'00'00; break;
    case '1': r |= 0b11'00'00'00; break;
    }
    return r;
}

template<char A, char B, char C, char D>
[[nodiscard]] constexpr static int f32x4_sse_zero_mask() noexcept
{
    int r = 0;
    r |= (A == '1') ? 0 : 0b0001;
    r |= (B == '1') ? 0 : 0b0010;
    r |= (C == '1') ? 0 : 0b0100;
    r |= (D == '1') ? 0 : 0b1000;
    return r;
}

template<char A, char B, char C, char D>
[[nodiscard]] constexpr static int f32x4_sse_number_mask() noexcept
{
    int r = 0;
    r |= (a == '0' || a == '1') ? 0b0001 : 0;
    r |= (b == '0' || b == '1') ? 0b0010 : 0;
    r |= (c == '0' || c == '1') ? 0b0100 : 0;
    r |= (d == '0' || d == '1') ? 0b1000 : 0;
    return r;
}

template<char A, char B, char C, char D>
[[nodiscard]] aligned_array<float,4> f32x4_swizzle(aligned_array<float,4> value) const noexcept
{
    constexpr int permute_mask = vec::swizzle_fvec4_sse_permute_mask<A, B, C, D>();
    constexpr int zero_mask = vec::swizzle_fvec4_sse_zero_mask<A, B, C, D>();
    constexpr int number_mask = vec::swizzle_fvec4_sse_number_mask<A, B, C, D>();

    __m128 swizzled;
    // Clang is able to optimize these intrinsics, MSVC is not.
    if constexpr (permute_mask != 0b11'10'01'00) {
        swizzled = _mm_permute_ps(to_m128(value), permute_mask);
    } else {
        swizzled = to_m128(value);
    }

    __m128 numbers;
    if constexpr (zero_mask == 0b0000) {
        numbers = _mm_set_ps1(1.0f);
    } else if constexpr (zero_mask == 0b1111) {
        numbers = _mm_setzero_ps();
    } else if constexpr (zero_mask == 0b1110) {
        numbers = _mm_set_ss(1.0f);
    } else {
        ttlet _1111 = _mm_set_ps1(1.0f);
        numbers = _mm_insert_ps(_1111, _1111, zero_mask);
    }

    __m128 result;
    if constexpr (number_mask == 0b0000) {
        result = swizzled;
    } else if constexpr (number_mask == 0b1111) {
        result = numbers;
    } else if constexpr (((zero_mask | ~number_mask) & 0b1111) == 0b1111) {
        result = _mm_insert_ps(swizzled, swizzled, number_mask);
    } else {
        result = _mm_blend_ps(swizzled, numbers, number_mask);
    }
    return to_aligned_array_f32x4(result);
}

}
