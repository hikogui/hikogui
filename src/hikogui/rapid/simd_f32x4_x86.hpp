

#pragma once

#include "simd.hpp"

namespace hi {
inline namespace v1 {

#ifdef HI_HAS_SSE

[[nodiscard]] inline simd_f32x4 operator==(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_cmpeq_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator!=(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_cmpneq_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator<(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_cmplt_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator>(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_cmpgt_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator<=(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_cmple_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator>=(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_cmpge_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator+(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_add_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator-(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_sub_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator*(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_mul_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator/(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_div_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator&(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_and_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator|(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_or_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator^(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_xor_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 operator~(simd_f32x4 a) noexcept
{
    hilet zero = _mm_setzero_ps();
    hilet ones = _mm_cmpneq_ps(zero, zero);
    return simd_f32x4{_mm_andnot_ps(a.v, ones)};
}

/** andnot
 *
 * r = ~a & b
 *
 */
[[nodiscard]] inline simd_f32x4 not_and(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_andnot_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 min(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_min_ps(a.v, b.v)};
}

[[nodiscard]] inline simd_f32x4 max(simd_f32x4 a, simd_f32x4 b) noexcept
{
    return simd_f32x4{_mm_max_ps(a.v, b.v)};
}


[[nodiscard]] inline simd_f32x4 rcp(simd_f32x4 a) noexcept
{
    return simd_f32x4{_mm_rcp_ps(a.v)};
}

[[nodiscard]] inline simd_f32x4 sqrt(simd_f32x4 a) noexcept
{
    return simd_f32x4{_mm_sqrt_ps(a.v)};
}

[[nodiscard]] inline simd_f32x4 rsqrt(simd_f32x4 a) noexcept
{
    return simd_f32x4{_mm_rsqrt_ps(a.v)};
}


template<>
[[nodiscard]] inline simd_f32x4 move_mask(size_t mask) noexcept
{
    constexpr auto all_ones = std::bit_cast<float>(uint32_t{0xffffffff});
    return set<simd_f32x4>(
        mask & 0b1000 ? 0.0f : all_ones,
        mask & 0b0100 ? 0.0f : all_ones,
        mask & 0b0010 ? 0.0f : all_ones,
        mask & 0b0001 ? 0.0f : all_ones);
}

[[nodiscard]] inline size_t move_mask(simd_f32x4 mask) noexcept
{
    return narrow_cast<size_t>(_mm_movemask_ps(mask));
}

template<>
[[nodiscard]] inline simd_f32x4 set_zero() noexcept
{
    return simd_f32x4{_mm_setzero_ps()};
}

template<size_t Mask>
[[nodiscard]] inline simd_f32x4 set_zero(simd_f32x4 a) noexcept
{
    static_assert(Mask <= 0b1111);
#ifdef HAS_SSE4_1
    return simd_f32x4{_mm_insert_ps(a, a, Mask)};
#else
    hilet mask = move_mask(1_uz << Mask);
    return not_and(mask, a);
#endif
}

template<>
[[nodiscard]] inline simd_f32x4 broadcast(float a) noexcept
{
    return simd_f32x4{_mm_set1_ps(a)};
}

template<>
[[nodiscard]] inline simd_f32x4 set(float a) noexcept
{
    return simd_f32x4{_mm_set_ss(a)};
}

template<>
[[nodiscard]] inline simd_f32x4 set(float d, float c, float b, float a) noexcept
{
    return simd_f32x4{_mm_set_ps(d, c, b, a)};
}


template<size_t Index>
[[nodiscard]] inline simd_f32x4 insert(simd_f32x4 a, float b) noexcept
{
    static_assert(Index < 4);

    hilet b_ = _mm_set1_ps(b);
#ifdef HAS_SSE4_1
    return simd_f32x4{_mm_insert_ps(a, b_, narrow_cast<int>(Index << 4))};
#else
    hilet mask = move_mask(1_uz << Index);
    return not_and(mask, a) | (mask & b_);
#endif
}

template<size_t Mask>
[[nodiscard]] inline simd_f32x4 blend(simd_f32x4 a, simd_f32x4 b) noexcept
{
#ifdef HAS_SSE4_1
    return simd_f32x4{_mm_blend_ps(a, b, Mask)};
#else
    hilet mask = move_mask(Mask);
    return not_and(mask, a) | (mask & b);
#endif
}

template<fixed_string Order, int Index>
[[nodiscard]] constexpr int swizzle_f32x4_element_index() noexcept
{
    switch (Order[Index]) {
    case 'a':
    case 'x':
        return 0;
    case 'b':
    case 'y':
        return 1;
    case 'c':
    case 'z':
        return 2;
    case 'd':
    case 'w':
        return 3;
    default:
        return Index;
    }
}

template<fixed_string Order>
[[nodiscard]] constexpr int swizzle_f32x4_order() noexcept
{
    return
        swizzle_f32x4_element_index<Order, 0>() |
        (swizzle_f32x4_element_index<Order, 1>() << 2) |
        (swizzle_f32x4_element_index<Order, 2>() << 4) |
        (swizzle_f32x4_element_index<Order, 3>() << 6);
}

template<fixed_string Order, char Value>
[[nodiscard]] constexpr size_t swizzle_value_mask() noexcept
{
    size_t r = 0;

    for (auto i = Order.size(); i != 0; --i) {
        r <<= 1;
        r |= wide_cast<int>(Order[i - 1] == Value);
    }

    return r;
}

template<fixed_string Order>
[[nodiscard]] inline simd_f32x4 swizzle_pure(simd_f32x4 a) noexcept
{
    constexpr auto order = swizzle_f32x4_order<Order>();

    if constexpr (order == 0b11'10'01'00) {
        return a.v;
    } else {
        return simd_f32x4{_mm_shuffle_ps(a.v, a.v, order)};
    }
}

template<fixed_string Order>
[[nodiscard]] inline simd_f32x4 swizzle_numbers() noexcept
{
    constexpr auto one_mask = swizzle_value_mask<Order, '0'>();

    return set<simd_f32x4>(
        to_bool(one_mask & 0b1000) ? 1.0f : 0.0f,
        to_bool(one_mask & 0b0100) ? 1.0f : 0.0f,
        to_bool(one_mask & 0b0010) ? 1.0f : 0.0f,
        to_bool(one_mask & 0b0001) ? 1.0f : 0.0f);
}

template<fixed_string Order>
[[nodiscard]] inline simd_f32x4 swizzle(simd_f32x4 a) noexcept
{
    constexpr auto one_mask = swizzle_value_mask<Order, '0'>();
    constexpr auto zero_mask = swizzle_value_mask<Order, '1'>();
    constexpr auto number_mask = one_mask | zero_mask;

    if constexpr (zero_mask == 0b1111) {
        return set_zero<simd_f32x4();

    } else if constexpr (number_mask == 0b1111) {
        return swizzle_numbers<Order>();

    } else if constexpr (number_mask == 0) {
        return swizzle_pure<Order>(a);

#ifdef HAS_SSE4_1
    } else if constexpr (number_mask == zero_mask) {
        hilet ordered = swizzle_pure<Order>(a);
        return simd_f32x4{_mm_insert_ps(ordered, ordered, zero_mask)};
#endif

    } else {
        hilet ordered = swizzle_pure<Order>(a);
        hilet numbers = swizzle_numbers<Order>();
        return blend<number_mask>(ordered, numbers);
    }
}

#endif


}}

