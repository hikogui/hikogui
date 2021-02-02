
#pragma once

#include "../aligned_array.hpp"

namespace tt {

/** Take the ceil for each of the elements in the SSE register.
 */
[[nodiscard]] inline f32x4_raw f32x4_sse_ceil(f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_ceil_ps(static_cast<__m128>(rhs))};
}

/** Take the floor for each of the elements in the SSE register.
 */
[[nodiscard]] inline f32x4_raw f32x4_sse_floor(f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_floor_ps(static_cast<__m128>(rhs))};
}

/** Round each of the elements in the current rounding direction in the SSE register.
 */
[[nodiscard]] inline f32x4_raw f32x4_sse_round(f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_round_ps(static_cast<__m128>(rhs), _MM_FROUND_CUR_DIRECTION)};
}

/** Take the reciprocal of each element in the SSE register.
 */
[[nodiscard]] inline f32x4_raw f32x4_sse_rcp(f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_rcp_ps(static_cast<__m128>(rhs))};
}

/** Clear elements of an SSE register.
 *
 * @tparam Mask '1': 0.0, '0': original value.
 */
template<unsigned int Mask>
[[nodiscard]] inline f32x4_raw f32x4_sse_clear(f32x4_raw const &rhs) noexcept
{
    static_assert(Mask ^ (Mask & 0xf) == 0);

    if constexpr (Mask == 0b0000) {
        return rhs;
    } else if constexpr (Mask == 0b1111) {
        // 1 cycle
        return f32x4_raw{_mm_setzero_ps()};
    } else {
        // 1 cycle
        return f32x4_raw{_mm_insert_ps(static_cast<__m128>(rhs), static_cast<__m128>(rhs), Mask)};
    }
}

/** Make sign bit-pattern for each element in the SSE register.
 * This function is used to XOR with floating point numbers to
 * toggle the sign.
 *
 * @tparam Mask '1': -0.0, '0': 0.0
 */
template<unsigned int Mask>
[[nodiscard]] inline f32x4_raw f32x4_sse_make_sign() noexcept
{
    static_assert(Mask ^ (Mask & 0xf) == 0);

    if constexpr (Mask == 0b0000) {
        return f32x4_raw{_mm_setzero_ps()};

    } else if constexpr (Mask == 0b0001) {
        return f32x4_raw{_mm_set_ss(-0.0f)};

    } else if constexpr (Mask == 0b1111) {
        return f32x4_raw{_mm_set_ps1(-0.0f)};

    } else {
        constexpr float x = (Mask & 0b0001) == 0 ? 0.0f : -0.0f;
        constexpr float y = (Mask & 0b0010) == 0 ? 0.0f : -0.0f;
        constexpr float z = (Mask & 0b0100) == 0 ? 0.0f : -0.0f;
        constexpr float w = (Mask & 0b1000) == 0 ? 0.0f : -0.0f;
        return f32x4_raw{_mm_set_epi32(w, z, y, x)};
    }
}

/** Negate elements in an SSE register.
 *
 * @tparam Mask '1': Negate element, '0': Original element
 */
template<unsigned int Mask>
[[nodiscard]] inline f32x4_raw f32x4_sse_neg(f32x4_raw const &rhs) noexcept
{
    static_assert(Mask ^ (Mask & 0xf) == 0);

    if constexpr (Mask == 0b0000) {
        return rhs;

    } else {
        ttlet sign = static_cast<__m128>(f32x4_sse_make_sign<Mask>());
        return f32x4_raw{_mm_xor_ps(static_cast<__m128>(rhs), sign)};
    }
}

/** Add elements horizontally together.
 * x = lhs.x + lhs.y
 * y = lhs.z + lhs.w
 * z = rhs.x + rhs.y
 * w = rhs.z + rhs.w
 */
[[nodiscard]] inline f32x4_raw
f32x4_sse_hadd(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_hadd_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs))};
}

/** Subtract elements horizontally together.
 * x = lhs.x - lhs.y
 * y = lhs.z - lhs.w
 * z = rhs.x - rhs.y
 * w = rhs.z - rhs.w
 */
[[nodiscard]] inline f32x4_raw
f32x4_sse_hsub(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_hsub_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs))};
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
[[nodiscard]] inline f32x4_raw f32x4_sse_addsub(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    static_assert(Mask ^ (Mask & 0xf) == 0, "Only bottom 4 lsb may be set");

    ttlet lhs_ = static_cast<__m128>(lhs);
    ttlet rhs_ = static_cast<__m128>(rhs);

    if constexpr (Mask == 0b0000) {
        return f32x4_raw{_mm_sub_ps(lhs_, rhs_)};

    } else if constexpr (Mask == 0b0101) {
        return f32x4_raw{_mm_addsub_ps(lhs_, rhs_)};

    } else if constexpr (Mask == 0b1010) {
        ttlet neg_rhs = static_cast<__m128>(f32x4_sse_neg<0b1111>(rhs));
        return f32x4_raw{_mm_addsub_ps(lhs_, neg_rhs)};
        
    } else if constexpr (Mask == 0b1111) {
        return f32x4_raw{_mm_add_ps(lhs_, rhs_)};

    } else {
        ttlet neg_rhs = static_cast<__m128>(f32x4_sse_neg<~Mask & 0xf>(rhs));
        return f32x4_raw{_mm_add_ps(lhs_, neg_rhs)};
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
[[nodiscard]] float f32x4_sse_dot(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    static_assert(Mask ^ (Mask & 0xf) == 0, "Only bottom 4 lsb may be set");
    constexpr int imm8 = (Mask << 4) | 0x1;

    auto tmp = f32x4_raw{_mm_dp_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs), imm8)};
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
[[nodiscard]] float f32x4_sse_hypot(f32x4_raw const &rhs) noexcept
{
    static_assert(Mask ^ (Mask & 0xf) == 0, "Only bottom 4 lsb may be set");
    constexpr int imm8 = (Mask << 4) | 0x1;

    auto _rhs = static_cast<__m128>(rhs);
    auto tmp = f32x4_raw{_mm_sqrt_ps(_mm_dp_ps(_rhs, _rhs, imm8))};
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
[[nodiscard]] float f32x4_sse_rcp_hypot(f32x4_raw const &rhs) noexcept
{
    static_assert(Mask ^ (Mask & 0xf) == 0, "Only bottom 4 lsb may be set");
    constexpr int imm8 = (Mask << 4) | 0x1;

    auto _rhs = static_cast<__m128>(rhs);
    auto tmp = f32x4_raw{_mm_rsqrt_ps(_mm_dp_ps(_rhs, _rhs, imm8))};
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
[[nodiscard]] f32x4_raw f32x4_sse_normalize(f32x4_raw const &rhs) noexcept
{
    static_assert(Mask ^ (Mask & 0xf) == 0, "Only bottom 4 lsb may be set");
    constexpr int imm8 = (Mask << 4) | Mask;

    auto rhs_ = static_cast<__m128>(rhs);
    auto rcp_length = f32x4_raw{_mm_rsqrt_ps(_mm_dp_ps(rhs_, rhs_, imm8))};
    return _mm_mul_ps(rhs_, rcp_length);
}

/** Compare if equal elements of two SSE registers and return a mask.
 */
[[nodiscard]] inline unsigned int
f32x4_sse_eq_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmpeq_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

/** Compare if not-equal elements of two SSE registers and return a mask.
 */
[[nodiscard]] inline unsigned int
f32x4_sse_ne_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmpneq_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

/** Compare if less-than elements of two SSE registers and return a mask.
 */
[[nodiscard]] inline unsigned int
f32x4_sse_lt_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmplt_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

/** Compare if greater-than elements of two SSE registers and return a mask.
 */
[[nodiscard]] inline unsigned int
f32x4_sse_gt_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmpgt_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

/** Compare if less-or-equal elements of two SSE registers and return a mask.
 */
[[nodiscard]] inline unsigned int
f32x4_sse_le_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmple_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

/** Compare if greater-or-equal elements of two SSE registers and return a mask.
 */
[[nodiscard]] inline unsigned int
f32x4_sse_ge_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmpge_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

/** Compare if both SSE registers are completely equal.
 */
[[nodiscard]] inline bool f32x4_sse_eq(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
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

    auto tmp = _mm_cmpneq_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return _mm_testz_ps(tmp, tmp);
}

/** Calculate a cross-product on two 2D vectors.
 *
 * The value returned is a single floating point value represents an angle, in some form.
 */
[[nodiscard]] inline float f32x4_sse_viktor_cross(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    // a.x * b.y - a.y * b.x
    ttlet tmp1 = _mm_permute_ps(static_cast<__m128>(rhs), _MM_SHUFFLE(2, 3, 0, 1));
    ttlet tmp2 = _mm_mul_ps(static_cast<__m128>(lhs), tmp1);
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
[[nodiscard]] inline f32x4_raw f32x4_sse_hamilton_cross(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    ttlet lhs_ = static_cast<__m128>(lhs);
    ttlet rhs_ = static_cast<__m128>(rhs);

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

    ttlet s0 = f32x4_sse_addsub<0b0101>(f32x4_raw{w}, f32x4_raw{x});
    ttlet s1 = f32x4_sse_addsub<0b0011>(s0, f32x4_raw{y});
    return f32x4_sse_addsub<0b0110>(sl, f32x4_raw{z});
}


/** 3D Cross produce between two vectors.
 *
 * x = y1*z2 - z1*y2
 * y = z1*x2 - x1*z2
 * z = x1*y2 - y1*x2
 * w = w1*w2 - w1*w2
 */
[[nodiscard]] inline f32x4_raw f32x4_sse_cross(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    ttlet a_left = _mm_permute_ps(static_cast<__m128>(lhs), _MM_SHUFFLE(3, 0, 2, 1));
    ttlet b_left = _mm_permute_ps(static_cast<__m128>(rhs), _MM_SHUFFLE(3, 1, 0, 2));
    ttlet left = _mm_mul_ps(a_left, b_left);

    ttlet a_right = _mm_permute_ps(static_cast<__m128>(lhs), _MM_SHUFFLE(3, 1, 0, 2));
    ttlet b_right = _mm_permute_ps(static_cast<__m128>(rhs), _MM_SHUFFLE(3, 0, 2, 1));
    ttlet right = _mm_mul_ps(a_right, b_right);
    return f32x4_raw{_mm_sub_ps(left, right)};
}

[[nodiscard]] inline std::array<f32x4_raw, 4> f32x4_sse_transpose(
    f32x4_raw const &col0,
    f32x4_raw const &col1,
    f32x4_raw const &col2,
    f32x4_raw const &col3) noexcept
{
    auto col0_ = static_cast<__m128>(col0);
    auto col1_ = static_cast<__m128>(col1);
    auto col2_ = static_cast<__m128>(col2);
    auto col3_ = static_cast<__m128>(col3);

    _MM_TRANSPOSE4_PS(col0_, col1_, col2_, col3_);

    return {
        f32x4_raw{col0_},
        f32x4_raw{col1_},
        f32x4_raw{col2_},
        f32x4_raw{col3_}};
}

template<ssize_t A, ssize_t B, ssize_t C, ssize_t D>
[[nodiscard]] constexpr static int f32x4_sse_permute_mask() noexcept
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
[[nodiscard]] constexpr static int f32x4_sse_not_one_mask() noexcept
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
[[nodiscard]] constexpr static int f32x4_sse_number_mask() noexcept
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
[[nodiscard]] f32x4_raw f32x4_sse_swizzle(f32x4_raw const &value) noexcept
{
    static_assert(A >= -3 && A < 4);
    static_assert(B >= -3 && B < 4);
    static_assert(C >= -3 && C < 4);
    static_assert(D >= -3 && D < 4);

    constexpr int permute_mask = f32x4_sse_permute_mask<A, B, C, D>();
    constexpr int not_one_mask = f32x4_sse_not_one_mask<A, B, C, D>();
    constexpr int number_mask = f32x4_sse_number_mask<A, B, C, D>();

    __m128 swizzled;
    // Clang is able to optimize these intrinsics, MSVC is not.
    if constexpr (permute_mask != 0b11'10'01'00) {
        swizzled = _mm_permute_ps(static_cast<__m128>(value), permute_mask);
    } else {
        swizzled = static_cast<__m128>(value);
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
    return f32x4_raw{result};
}

}
