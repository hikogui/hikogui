// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "raw_numeric_array.hpp"

#include <array>
#include <emmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>

namespace tt {

/** Convert integers to floats.
 */
[[nodiscard]] inline rf32x4 f32x4_x64v2_from_i32x4(ri32x4 const &rhs) noexcept
{
    return to_rf32x4(_mm_cvtepi32_ps(to_m128i(rhs)));
}

/** Clear elements of an SSE register.
 *
 * @tparam Mask '1': 0.0, '0': original value.
 */
template<unsigned int Mask>
[[nodiscard]] inline rf32x4 f32x4_x64v2_clear(rf32x4 const &rhs) noexcept
{
    static_assert((Mask ^ (Mask & 0xf)) == 0);

    if constexpr (Mask == 0b0000) {
        return rhs;
    } else if constexpr (Mask == 0b1111) {
        // 1 cycle
        return to_rf32x4(_mm_setzero_ps());
    } else {
        // 1 cycle
        return to_rf32x4(_mm_insert_ps(to_m128(rhs), to_m128(rhs), Mask));
    }
}

/** Make sign bit-pattern for each element in the SSE register.
 * This function is used to XOR with floating point numbers to
 * toggle the sign.
 *
 * @tparam Mask '1': -0.0, '0': 0.0
 */
template<unsigned int Mask>
[[nodiscard]] inline rf32x4 f32x4_x64v2_make_sign() noexcept
{
    static_assert((Mask ^ (Mask & 0xf)) == 0);

    if constexpr (Mask == 0b0000) {
        return to_rf32x4(_mm_setzero_ps());

    } else if constexpr (Mask == 0b0001) {
        return to_rf32x4(_mm_set_ss(-0.0f));

    } else if constexpr (Mask == 0b1111) {
        return to_rf32x4(_mm_set_ps1(-0.0f));

    } else {
        constexpr float x = (Mask & 0b0001) == 0 ? 0.0f : -0.0f;
        constexpr float y = (Mask & 0b0010) == 0 ? 0.0f : -0.0f;
        constexpr float z = (Mask & 0b0100) == 0 ? 0.0f : -0.0f;
        constexpr float w = (Mask & 0b1000) == 0 ? 0.0f : -0.0f;
        return to_rf32x4(_mm_set_ps(w, z, y, x));
    }
}

/** Negate elements in an SSE register.
 *
 * @tparam Mask '1': Negate element, '0': Original element
 */
template<unsigned int Mask>
[[nodiscard]] inline rf32x4 f32x4_x64v2_neg(rf32x4 const &rhs) noexcept
{
    static_assert((Mask ^ (Mask & 0xf)) == 0);

    if constexpr (Mask == 0b0000) {
        return rhs;

    } else {
        ttlet sign = to_m128(f32x4_x64v2_make_sign<Mask>());
        return to_rf32x4(_mm_xor_ps(to_m128(rhs), sign));
    }
}


/** Add or subtract elements of two SSE registers.
 * This function is useful for creating cross products and generating a matrix
 * from a quarternion where you have a mix of adds and subtracts.
 *
 * Clock cycles:
 *      L:3  T:1   _mm_sub_ps (Mask=0000)
 *
 *      L:3  T:1   _mm_add_ps (Mask=1111)
 *
 *      L:3  T:1   _mm_addsub_ps (Mask=0101)
 *
 *      L:1  T:0.5 _mm_set_epi32 (Mask=1010)
 *      L:1  T:1   _mm_xor_ps
 *      L:3  T:1   _mm_addsub_ps
 *
 *      L:1  T:0.5 _mm_set_epi32 (Mask=other)
 *      L:1  T:1   _mm_xor_ps
 *      L:3  T:1   _mm_add_ps
 *
 * @tparam Mask '1' add, '0' subtract.
 */
template<unsigned int Mask>
[[nodiscard]] inline rf32x4 f32x4_x64v2_addsub(rf32x4 const &lhs, rf32x4 const &rhs) noexcept
{
    static_assert((Mask ^ (Mask & 0xf)) == 0, "Only bottom 4 lsb may be set");

    ttlet lhs_ = to_m128(lhs);
    ttlet rhs_ = to_m128(rhs);

    if constexpr (Mask == 0b0000) {
        return to_rf32x4(_mm_sub_ps(lhs_, rhs_));

    } else if constexpr (Mask == 0b0101) {
        return to_rf32x4(_mm_addsub_ps(lhs_, rhs_));

    } else if constexpr (Mask == 0b1010) {
        ttlet neg_rhs = to_m128(f32x4_x64v2_neg<0b1111>(rhs));
        return to_rf32x4(_mm_addsub_ps(lhs_, neg_rhs));

    } else if constexpr (Mask == 0b1111) {
        return to_rf32x4(_mm_add_ps(lhs_, rhs_));

    } else {
        ttlet neg_rhs = to_m128(f32x4_x64v2_neg<~Mask & 0xf>(rhs));
        return to_rf32x4(_mm_add_ps(lhs_, neg_rhs));
    }
}

/** Get the dot product of two SSE registers.
 *
 * Clock cycles:
 *      L:11 T:1.5 _mm_dp_ps
 *
 * @tparam Mask '1': include element in dot product, '0': do not include element in dot product
 */
template<unsigned int Mask>
[[nodiscard]] float f32x4_x64v2_dot(rf32x4 const &lhs, rf32x4 const &rhs) noexcept
{
    static_assert((Mask ^ (Mask & 0xf)) == 0, "Only bottom 4 lsb may be set");
    constexpr int imm8 = (Mask << 4) | 0x1;

    auto tmp = to_rf32x4(_mm_dp_ps(to_m128(lhs), to_m128(rhs), imm8));
    return get<0>(tmp);
}

/** Get the hypot of a SSE register.
 *
 * Clock cycles:
 *      L:11 T:1.5 _mm_dp_ps
 *      L:13 T:7   _mm_sqrt_ps
 *
 * @tparam Mask '1': include element in the hypot, '0': do not include element in the hypot
 */
template<unsigned int Mask>
[[nodiscard]] float f32x4_x64v2_hypot(rf32x4 const &rhs) noexcept
{
    static_assert((Mask ^ (Mask & 0xf)) == 0, "Only bottom 4 lsb may be set");
    constexpr int imm8 = (Mask << 4) | 0x1;

    auto _rhs = to_m128(rhs);
    auto tmp = to_rf32x4(_mm_sqrt_ps(_mm_dp_ps(_rhs, _rhs, imm8)));
    return get<0>(tmp);
}

/** Get the reciprocal of the hypot of a SSE register.
 *
 * Clock cycles:
 *      L:11 T:1.5 _mm_dp_ps
 *      L:5  T:1   _mm_rsqrt_ps
 *
 * @tparam Mask '1': include element in the hypot, '0': do not include element in the hypot
 */
template<unsigned int Mask>
[[nodiscard]] float f32x4_x64v2_rcp_hypot(rf32x4 const &rhs) noexcept
{
    static_assert((Mask ^ (Mask & 0xf)) == 0, "Only bottom 4 lsb may be set");
    constexpr int imm8 = (Mask << 4) | 0x1;

    auto _rhs = to_m128(rhs);
    auto tmp = to_rf32x4(_mm_rsqrt_ps(_mm_dp_ps(_rhs, _rhs, imm8)));
    return get<0>(tmp);
}

/** Get normalize a vector encoded in a SSE register.
 *
 * Clock cycles:
 *      L:11 T:1.5 _mm_dp_ps
 *      L:5  T:1   _mm_rsqrt_ps
 *      L:4  T:0.5 _mm_mul_ps
 *
 * @tparam Mask '1': include element in the normalization, '0': Do not include and set element to zero
 */
template<unsigned int Mask>
[[nodiscard]] rf32x4 f32x4_x64v2_normalize(rf32x4 const &rhs) noexcept
{
    static_assert((Mask ^ (Mask & 0xf)) == 0, "Only bottom 4 lsb may be set");
    constexpr int dp_imm8 = (Mask << 4) | Mask;
    constexpr int zero_imm8 = ~Mask & 0xf;

    ttlet rhs_ = to_m128(rhs);
    ttlet rcp_length = _mm_rsqrt_ps(_mm_dp_ps(rhs_, rhs_, dp_imm8));
    ttlet rcp_length_ = _mm_insert_ps(rcp_length, rcp_length, zero_imm8);
    return to_rf32x4(_mm_mul_ps(rhs_, rcp_length_));
}

/** Compare if both SSE registers are completely equal.
 */
[[nodiscard]] inline bool f32x4_x64v2_eq(rf32x4 const &lhs, rf32x4 const &rhs) noexcept
{
    // Example 1: lhs == rhs
    //    tmp -> (1.0, 1.0, 1.0, 1.0) != (1.0, 1.0, 1.0, 1.0) -> (0,0,0,0)
    //    return -> x == 0 && y == 0 && z == 0 && w == 0 -> true

    // Example 2: lhs != rhs
    //    tmp -> (0.0, 1.0, 1.0, 1.0) != (1.0, 1.0, 1.0, 1.0) -> (1,0,0,0)
    //    return -> x == 0 && y == 0 && z == 0 && w == 0 -> false

    // Example 3: lhs != rhs
    //    tmp -> (0.0, 0.0, 0.0, 0.0) != (1.0, 1.0, 1.0, 1.0) -> (1,1,1,1)
    //    return -> x == 0 && y == 0 && z == 0 && w == 0 -> false

    auto tmp = _mm_cmpneq_ps(to_m128(lhs), to_m128(rhs));
    return _mm_testz_ps(tmp, tmp);
}

/** Calculate a cross-product on two 2D vectors.
 *
 * The value returned is a single floating point value represents an angle, in some form.
 */
[[nodiscard]] inline float f32x4_x64v2_viktor_cross(rf32x4 const &lhs, rf32x4 const &rhs) noexcept
{
    // a.x * b.y - a.y * b.x
    ttlet tmp1 = _mm_permute_ps(to_m128(rhs), _MM_SHUFFLE(2, 3, 0, 1));
    ttlet tmp2 = _mm_mul_ps(to_m128(lhs), tmp1);
    ttlet tmp3 = _mm_hsub_ps(tmp2, tmp2);
    return _mm_cvtss_f32(tmp3);
}

/** 4D Quarternion cross product
 * x*i + y*j + z*k + w
 *
 * x = w1*x2 + x1*w2 + y1*z2 - z1*y2
 * y = w1*y2 - x1*z2 + y1*w2 + z1*x2
 * z = w1*z2 + x1*y2 - y1*x2 + z1*w2
 * w = w1*w2 - x1*x2 - y1*y2 - z1*z2
 */
[[nodiscard]] inline rf32x4 f32x4_x64v2_hamilton_cross(rf32x4 const &lhs, rf32x4 const &rhs) noexcept
{
    ttlet lhs_ = to_m128(lhs);
    ttlet rhs_ = to_m128(rhs);

    ttlet lhs_x = _mm_permute_ps(lhs_, _MM_SHUFFLE(0, 0, 0, 0));
    ttlet lhs_y = _mm_permute_ps(lhs_, _MM_SHUFFLE(1, 1, 1, 1));
    ttlet lhs_z = _mm_permute_ps(lhs_, _MM_SHUFFLE(2, 2, 2, 2));
    ttlet lhs_w = _mm_permute_ps(lhs_, _MM_SHUFFLE(3, 3, 3, 3));

    ttlet rhs_1 = _mm_permute_ps(rhs_, _MM_SHUFFLE(0, 1, 2, 3));
    ttlet rhs_2 = _mm_permute_ps(rhs_, _MM_SHUFFLE(1, 0, 3, 2));
    ttlet rhs_3 = _mm_permute_ps(rhs_, _MM_SHUFFLE(2, 3, 0, 1));

    ttlet w = _mm_mul_ps(lhs_w, rhs_);
    ttlet x = _mm_mul_ps(lhs_x, rhs_1);
    ttlet y = _mm_mul_ps(lhs_y, rhs_2);
    ttlet z = _mm_mul_ps(lhs_z, rhs_3);

    ttlet s0 = f32x4_x64v2_addsub<0b0101>(to_rf32x4(w), to_rf32x4(x));
    ttlet s1 = f32x4_x64v2_addsub<0b0011>(s0, to_rf32x4(y));
    return f32x4_x64v2_addsub<0b0110>(s1, to_rf32x4(z));
}

/** 3D Cross produce between two vectors.
 *
 * x = y1*z2 - z1*y2
 * y = z1*x2 - x1*z2
 * z = x1*y2 - y1*x2
 * w = w1*w2 - w1*w2
 */
[[nodiscard]] inline rf32x4 f32x4_x64v2_cross(rf32x4 const &lhs, rf32x4 const &rhs) noexcept
{
    ttlet a_left = _mm_permute_ps(to_m128(lhs), _MM_SHUFFLE(3, 0, 2, 1));
    ttlet b_left = _mm_permute_ps(to_m128(rhs), _MM_SHUFFLE(3, 1, 0, 2));
    ttlet left = _mm_mul_ps(a_left, b_left);

    ttlet a_right = _mm_permute_ps(to_m128(lhs), _MM_SHUFFLE(3, 1, 0, 2));
    ttlet b_right = _mm_permute_ps(to_m128(rhs), _MM_SHUFFLE(3, 0, 2, 1));
    ttlet right = _mm_mul_ps(a_right, b_right);
    return to_rf32x4(_mm_sub_ps(left, right));
}

[[nodiscard]] inline std::array<rf32x4, 4>
f32x4_x64v2_transpose(rf32x4 const &col0, rf32x4 const &col1, rf32x4 const &col2, rf32x4 const &col3) noexcept
{
    auto col0_ = to_m128(col0);
    auto col1_ = to_m128(col1);
    auto col2_ = to_m128(col2);
    auto col3_ = to_m128(col3);

    _MM_TRANSPOSE4_PS(col0_, col1_, col2_, col3_);

    return {to_rf32x4(col0_), to_rf32x4(col1_), to_rf32x4(col2_), to_rf32x4(col3_)};
}

template<ssize_t A, ssize_t B, ssize_t C, ssize_t D>
[[nodiscard]] constexpr static int f32x4_x64v2_permute_mask() noexcept
{
    static_assert(A >= -3 && A < 4);
    static_assert(B >= -3 && B < 4);
    static_assert(C >= -3 && C < 4);
    static_assert(D >= -3 && D < 4);

    int r = 0;
    switch (A) {
    case 0: r |= 0b00'00'00'00; break;
    case 1: r |= 0b00'00'00'01; break;
    case 2: r |= 0b00'00'00'10; break;
    case 3: r |= 0b00'00'00'11; break;
    case -1: r |= 0b00'00'00'00; break;
    case -2: r |= 0b00'00'00'00; break;
    }
    switch (B) {
    case 0: r |= 0b00'00'00'00; break;
    case 1: r |= 0b00'00'01'00; break;
    case 2: r |= 0b00'00'10'00; break;
    case 3: r |= 0b00'00'11'00; break;
    case -1: r |= 0b00'00'01'00; break;
    case -2: r |= 0b00'00'01'00; break;
    }
    switch (C) {
    case 0: r |= 0b00'00'00'00; break;
    case 1: r |= 0b00'01'00'00; break;
    case 2: r |= 0b00'10'00'00; break;
    case 3: r |= 0b00'11'00'00; break;
    case -1: r |= 0b00'10'00'00; break;
    case -2: r |= 0b00'10'00'00; break;
    }
    switch (D) {
    case 0: r |= 0b00'00'00'00; break;
    case 1: r |= 0b01'00'00'00; break;
    case 2: r |= 0b10'00'00'00; break;
    case 3: r |= 0b11'00'00'00; break;
    case -1: r |= 0b11'00'00'00; break;
    case -2: r |= 0b11'00'00'00; break;
    }
    return r;
}

template<ssize_t A, ssize_t B, ssize_t C, ssize_t D>
[[nodiscard]] constexpr static int f32x4_x64v2_not_one_mask() noexcept
{
    static_assert(A >= -3 && A < 4);
    static_assert(B >= -3 && B < 4);
    static_assert(C >= -3 && C < 4);
    static_assert(D >= -3 && D < 4);

    int r = 0;
    r |= (A == -2) ? 0 : 0b0001;
    r |= (B == -2) ? 0 : 0b0010;
    r |= (C == -2) ? 0 : 0b0100;
    r |= (D == -2) ? 0 : 0b1000;
    return r;
}

template<ssize_t A, ssize_t B, ssize_t C, ssize_t D>
[[nodiscard]] constexpr static int f32x4_x64v2_number_mask() noexcept
{
    static_assert(A >= -3 && A < 4);
    static_assert(B >= -3 && B < 4);
    static_assert(C >= -3 && C < 4);
    static_assert(D >= -3 && D < 4);

    int r = 0;
    r |= A < 0 ? 0b0001 : 0;
    r |= B < 0 ? 0b0010 : 0;
    r |= C < 0 ? 0b0100 : 0;
    r |= D < 0 ? 0b1000 : 0;
    return r;
}

template<ssize_t A = -1, ssize_t B = -1, ssize_t C = -1, ssize_t D = -1>
[[nodiscard]] rf32x4 f32x4_x64v2_swizzle(rf32x4 const &value) noexcept
{
    static_assert(A >= -3 && A < 4);
    static_assert(B >= -3 && B < 4);
    static_assert(C >= -3 && C < 4);
    static_assert(D >= -3 && D < 4);

    constexpr int permute_mask = f32x4_x64v2_permute_mask<A, B, C, D>();
    constexpr int not_one_mask = f32x4_x64v2_not_one_mask<A, B, C, D>();
    constexpr int number_mask = f32x4_x64v2_number_mask<A, B, C, D>();

    __m128 swizzled;
    // Clang is able to optimize these intrinsics, MSVC is not.
    if constexpr (permute_mask != 0b11'10'01'00) {
        swizzled = _mm_permute_ps(to_m128(value), permute_mask);
    } else {
        swizzled = to_m128(value);
    }

    __m128 numbers;
    if constexpr (not_one_mask == 0b0000) {
        numbers = _mm_set_ps1(1.0f);
    } else if constexpr (not_one_mask == 0b1111) {
        numbers = _mm_setzero_ps();
    } else if constexpr (not_one_mask == 0b1110) {
        numbers = _mm_set_ss(1.0f);
    } else {
        ttlet _1111 = _mm_set_ps1(1.0f);
        numbers = _mm_insert_ps(_1111, _1111, not_one_mask);
    }

    __m128 result;
    if constexpr (number_mask == 0b0000) {
        result = swizzled;
    } else if constexpr (number_mask == 0b1111) {
        result = numbers;
    } else if constexpr (((not_one_mask | ~number_mask) & 0b1111) == 0b1111) {
        result = _mm_insert_ps(swizzled, swizzled, number_mask);
    } else {
        result = _mm_blend_ps(swizzled, numbers, number_mask);
    }
    return to_rf32x4(result);
}

template<ssize_t A = -1, ssize_t B = -1>
[[nodiscard]] ru64x2 u64x2_x64v2_swizzle(ru64x2 const &value) noexcept
{
    constexpr auto A1 = A >= 0 ? A * 2 : A;
    constexpr auto A2 = A >= 0 ? A1 + 1 : A1;
    constexpr auto B1 = B >= 0 ? B * 2 : B;
    constexpr auto B2 = B >= 0 ? B1 + 1 : B1;

    ttlet value_ = _mm_castsi128_ps(to_m128i(value));
    ttlet r = to_m128(f32x4_x64v2_swizzle<A1, A2, B1, B2>(to_rf32x4(value_)));
    return to_ru64x2(_mm_castps_si128(r));
}

template<size_t FromElement, size_t ToElement, size_t ZeroMask>
[[nodiscard]] rf32x4 f32x4_x64v2_insert(rf32x4 const &lhs, rf32x4 const &rhs) noexcept
{
    static_assert(FromElement < 4);
    static_assert(ToElement < 4);
    static_assert(ZeroMask < 16);

    constexpr uint8_t insert_mask = static_cast<uint8_t>((FromElement << 6) | (ToElement << 4) | ZeroMask);

    return to_rf32x4(_mm_insert_ps(to_m128(lhs), to_m128(rhs), insert_mask));
}

template<size_t FromElement, size_t ToElement, size_t ZeroMask>
[[nodiscard]] ru64x2 u64x2_x64v2_insert(ru64x2 const &lhs, ru64x2 const &rhs) noexcept
{
    static_assert(FromElement < 2);
    static_assert(ToElement < 2);
    static_assert(ZeroMask < 4);

    if constexpr (ZeroMask == 0) {
        auto lhs_ = _mm_castsi128_pd(to_m128i(lhs));
        auto rhs_ = _mm_castsi128_pd(to_m128i(rhs));

        __m128d r;
        if constexpr (FromElement == 0 and ToElement == 0) {
            r = _mm_shuffle_pd(rhs_, lhs_, 0b10);
        } else if constexpr (FromElement == 1 and ToElement == 0) {
            r = _mm_shuffle_pd(rhs_, lhs_, 0b11);
        } else if constexpr (FromElement == 0 and ToElement == 1) {
            r = _mm_shuffle_pd(lhs_, rhs_, 0b00);
        } else {
            r = _mm_shuffle_pd(lhs_, rhs_, 0b10);
        }

        return to_ru64x2(_mm_castpd_si128(r));

    } else {
        constexpr size_t FromElement1 = FromElement * 2;
        constexpr size_t FromElement2 = FromElement1 + 1;
        constexpr size_t ToElement1 = ToElement * 2;
        constexpr size_t ToElement2 = ToElement1 + 1;
        constexpr size_t ZeroMask2 = (ZeroMask & 1 ? 0b11 : 0b00) | (ZeroMask & 2 ? 0b1100 : 0b0000);

        ttlet lhs_ = to_rf32x4(_mm_castsi128_ps(to_m128i(lhs)));
        ttlet rhs_ = to_rf32x4(_mm_castsi128_ps(to_m128i(rhs)));
        ttlet tmp = f32x4_x64v2_insert<FromElement1, ToElement1, 0>(lhs_, rhs_);
        ttlet r = f32x4_x64v2_insert<FromElement2, ToElement2, ZeroMask2>(tmp, rhs_);

        return to_ru64x2(_mm_castps_si128(to_m128(r)));
    }
}

} // namespace tt
