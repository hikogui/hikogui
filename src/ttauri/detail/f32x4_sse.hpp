
#pragma once

#include "../aligned_array.hpp"

namespace tt {

[[nodiscard]] inline f32x4_raw f32x4_sse_ceil(f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_ceil_ps(static_cast<__m128>(rhs))};
}

[[nodiscard]] inline f32x4_raw f32x4_sse_floor(f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_floor_ps(static_cast<__m128>(rhs))};
}

[[nodiscard]] inline f32x4_raw f32x4_sse_round(f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_round_ps(static_cast<__m128>(rhs), _MM_FROUND_CUR_DIRECTION)};
}

[[nodiscard]] inline f32x4_raw f32x4_sse_rcp(f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_rcp_ps(static_cast<__m128>(rhs))};
}


[[nodiscard]] inline f32x4_raw
f32x4_sse_hadd(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_hadd_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs))};
}

[[nodiscard]] inline f32x4_raw
f32x4_sse_hsub(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    return f32x4_raw{_mm_hsub_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs))};
}

template<size_t D>
[[nodiscard]] float f32x4_sse_dot(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    static_assert(D <= 4);
    constexpr int imm8 = 0xff >> (4 - D);

    auto tmp = f32x4_raw{_mm_dp_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs), imm8)};
    return get<0>(tmp);
}

template<size_t D>
[[nodiscard]] float f32x4_sse_hypot(f32x4_raw const &rhs) noexcept
{
    static_assert(D <= 4);
    constexpr int imm8 = 0xff >> (4 - D);

    auto _rhs = static_cast<__m128>(rhs);
    auto tmp = f32x4_raw{_mm_sqrt_ps(_mm_dp_ps(_rhs, _rhs, imm8))};
    return get<0>(tmp);
}

template<size_t D>
[[nodiscard]] float f32x4_sse_rcp_hypot(f32x4_raw const &rhs) noexcept
{
    static_assert(D <= 4);
    constexpr int imm8 = 0xff >> (4 - D);

    auto _rhs = static_cast<__m128>(rhs);
    auto tmp = f32x4_raw{_mm_rsqrt_ps(_mm_dp_ps(_rhs, _rhs, imm8))};
    return get<0>(tmp);
}

[[nodiscard]] inline unsigned int
f32x4_sse_eq_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmpeq_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

[[nodiscard]] inline unsigned int
f32x4_sse_ne_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmpneq_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

[[nodiscard]] inline unsigned int
f32x4_sse_lt_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmplt_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

[[nodiscard]] inline unsigned int
f32x4_sse_gt_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmpgt_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

[[nodiscard]] inline unsigned int
f32x4_sse_le_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmple_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

[[nodiscard]] inline unsigned int
f32x4_sse_ge_mask(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
{
    auto tmp = _mm_cmpge_ps(static_cast<__m128>(lhs), static_cast<__m128>(rhs));
    return static_cast<unsigned int>(_mm_movemask_ps(tmp));
}

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

[[nodiscard]] inline float f32x4_sse_viktor_cross(f32x4_raw const &lhs, f32x4_raw const &rhs)
{
    // a.x * b.y - a.y * b.x
    ttlet tmp1 = _mm_permute_ps(static_cast<__m128>(rhs), _MM_SHUFFLE(2, 3, 0, 1));
    ttlet tmp2 = _mm_mul_ps(static_cast<__m128>(lhs), tmp1);
    ttlet tmp3 = _mm_hsub_ps(tmp2, tmp2);
    return _mm_cvtss_f32(tmp3);
}

[[nodiscard]] inline f32x4_raw
f32x4_sse_cross(f32x4_raw const &lhs, f32x4_raw const &rhs) noexcept
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
