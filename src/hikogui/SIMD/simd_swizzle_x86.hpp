// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <bit>
#include <utility>

#pragma once

hi_export_module(hikogui.simd.simd_swizzle);

namespace hi { inline namespace v1 {
namespace detail {

template<int I, int Needle, int First, int... Rest>
constexpr void _make_swizzle_mask(size_t& mask) noexcept
{
    if constexpr (Needle == First) {
        mask |= 1 << I;
    }

    if constexpr (sizeof...(Rest) != 0) {
        _make_swizzle_mask<I + 1, Needle, Rest...>(mask);
    }
}

/** Get a mask for each element that has a specific index value.
 *
 * @tparma Needle The value of the index for which the mask bit is set to '1'.
 * @tparam Indices A set of indices.
 * @return A mask where each '1' corresponds to a @a Indices that matches @a Needle.
 */
template<int Needle, int... Indices>
[[nodiscard]] constexpr size_t make_swizzle_mask() noexcept
{
    auto r = size_t{};
    _make_swizzle_mask<0, Needle, Indices...>();
    return r;
}

template<size_t I, size_t NumElements, int First, int... Rest>
constexpr void _make_swizzle_packed_indices(size_t& packed_indices) noexcept
{
    static_assert(std::has_single_bit(NumElements));
    static_assert(std::cmp_less(I, NumElements));
    static_assert(std::cmp_less(First, NumElements));

    // Use the original order for elements which are literals.
    constexpr auto index = First < 0 ? I : First;
    constexpr auto index_width = std::bit_width(NumElements - 1);
    constexpr auto shifted_index = index << (I * index_width);

    packed_indices |= shifted_index;

    if constexpr (sizeof...(Indices) != 0) {
        _make_swizzle_packed_indices<I + 1, NumElements, Rest...>(packed_indices);
    }
}

/** Create a packed index to use as argument to simd swizzle instructions.
 *
 * @tparam NumElements Number of elements in the vector.
 * @tparam Indices... The indices for each element to elements.
 * @return A packed set of indices.
 */
template<size_t NumElements, int... Indices>
[[nodiscard]] constexpr size_t make_swizzle_packed_indices() noexcept
{
    static_assert(std::has_single_bit(NumElements));
    static_assert(sizeof...(Indices) == NumElements);

    auto r = size_t{};
    _make_swizzle_packed_indices(r);
    return r;
}

} // namespace detail

template<typename T, size_t N>
struct simd_swizzle;

#if defined(HI_HAS_SSE4_1)
template<>
struct simd_swizzle<float, 4> {
    using array_type = std::array<float, 4>;
    template<int... Indices>
    [[nodiscard]] array_type operator()(array_type const& lhs) const noexcept
    {
        constexpr auto zero_mask = detail::make_swizzle_mask<-1, Indices...>();
        constexpr auto one_mask = detail::make_swizzle_mask<-2, Indices...>();
        constexpr auto number_mask = zero_mask | one_mask;
        constexpr auto indices = detail::make_swizzle_packed_indices<4, Indices...>();

        // Short cut all literals.
        if constexpr (zero_mask == 0b1111) {
            return _mm_setzero_ps();
        } else if constexpr (one_mask == 0b1111) {
            return _mm_set1_ps(1.0f);
        } else if constexpr (number_mask == 0b1111) {
            return _mm_set_ps(
                one_mask & 0b1000 ? 1.0f : 0.0f,
                one_mask & 0b0100 ? 1.0f : 0.0f,
                one_mask & 0b0010 ? 1.0f : 0.0f,
                one_mask & 0b0001 ? 1.0f : 0.0f);
        }

        auto r = simd_load<float, 4>{}(lhs);
        if constexpr (indices != 0b11'10'01'00) {
            r = _mm_shuffle_ps(r, r, indices);
        }

        if constexpr (number_mask != 0) {
            auto tmp = _mm_setzero_ps();

            if constexpr (one_mask != 0) {
                tmp = _mm_blend_ps(tmp, _mm_set1_ps(1.0f), one_mask);
            }

            r = _mm_blend_ps(r, tmp, number_mask);
        }

        return simd_store<float, 4>{}(r);
    }
};

template<std::integral T>
    requires(sizeof(T) == 4)
struct simd_swizzle<T, 4> {
    using array_type = std::array<T, 4>;
    template<int... Indices>
    [[nodiscard]] array_type operator()(array_type const& lhs) const noexcept
    {
        constexpr auto zero_mask = detail::make_swizzle_mask<-1, Indices...>();
        constexpr auto one_mask = detail::make_swizzle_mask<-2, Indices...>();
        constexpr auto number_mask = zero_mask | one_mask;
        constexpr auto indices = detail::make_swizzle_packed_indices<4, Indices...>();

        auto r = simd_load<float, 4>{}(lhs);

        // Short cut if all elements are literals.
        if constexpr (number_mask == 0b1111) {
            auto tmp = simd_set_zero<T, 4>{}(r);

            if constexpr (one_mask != 0) {
                tmp = _mm_blend_epi32(tmp, simd_set_one<T, 4>{}(tmp), one_mask);
            }

            return simd_store<float, 4>{}(r);
        }

        if constexpr (indices != 0b11'10'01'00) {
            r = _mm_shuffle_ps(r, r, indices);
        }

        if constexpr (number_mask != 0) {
            auto tmp = simd_set_zero<T, 4>{}(r);

            if constexpr (one_mask != 0) {
                tmp = _mm_blend_ps(tmp, simd_set_one<T, 4>{}(tmp), one_mask);
            }

            r = _mm_blend_ps(r, tmp, number_mask);
        }

        return simd_store<float, 4>{}(r);
    }
};
#endif

}} // namespace hi::v1
