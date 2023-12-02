
#pragma once


hi_export_module(hikogui.simd.shuffle);

hi_export namespace hi { inline namespace v1 {
namespace detail {

template<size_t I, size_t NumElements, int First, int... Rest>
constexpr void _make_shuffle_indices_imm(size_t& packed_indices) noexcept
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
        _make_shuffle_indices_imm<I + 1, NumElements, Rest...>(packed_indices);
    }
}

/** Create a packed index to use as argument to simd swizzle instructions.
 *
 * @tparam NumElements Number of elements in the vector.
 * @tparam Indices... The indices for each element to elements.
 * @return A packed set of indices.
 */
template<size_t NumElements, int... Indices>
[[nodiscard]] constexpr size_t make_shuffle_indices_imm() noexcept
{
    static_assert(std::has_single_bit(NumElements));
    static_assert(sizeof...(Indices) == NumElements);

    auto r = size_t{};
    _make_shuffle_indices_imm(r);
    return r;
}

template<typename T, size_t N, size_t I, int First, int... Rest>
constexpr void _make_shuffle_indices_xvar(std::array<T, N> &r) noexcept
{
    if constexpr (First < 0) {
        std::get<I>(r) = static_cast<T>(I);
    } else {
        std::get<I>(r) = static_cast<T>(First);
    }

    if constexpr (sizeof...(Rest) != 0) {
        _make_shuffle_indices_xvar<T, N, I + 1, Rest...>(r);
    }
}

template<typename T, size_t N, int... Indices>
[[nodiscard]] hi_force_inline simd_reg_t<T, N> make_shuffle_indices_xvar() noexcept
{
    static_assert(sizeof...(Indices) == N);

    auto r = std::array<T, N>{};
    _make_shuffle_indices_xvar<T, N, 0, Indices...>(r);
    return simd_load<T, N>{}(r);
}

}

template<typename T, size_t N>
struct simd_shuffle;


#if defined(HI_HAS_SSE)
template<>
struct simd_shuffle<float, 4> {
    using reg_type = simd_reg_t<float, 4>;

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg_type operator()(reg_type a) const noexcept
    {
        constexpr auto indices = detail::make_shuffle_indices_imm<4, Indices...>();
        return _mm_shuffle_ps(a, a, indices);
    }
};
#endif

#if defined(HI_HAS_SSE2)
template<>
struct simd_shuffle<double, 2> {
    using reg_type = simd_reg_t<double, 2>;

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg_type operator()(reg_type a) const noexcept
    {
        constexpr auto indices = detail::make_shuffle_indices_imm<2, Indices...>();
        return _mm_shuffle_pd(a, a, indices);
    }
};

template<same_as_any<int64_t,uint64_t> T>
struct simd_shuffle<T, 2> {
    using reg_type = simd_reg_t<T, 2>;

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg_type operator()(reg_type a) const noexcept
    {
        constexpr auto indices64 = detail::make_shuffle_indices_imm<2, Indices...>();

        // clang-format off
        constexpr auto indices32 =
            (indices64 & 0b01 ? 0b00'00'11'10 : 0b00'00'01'00) |
            (indices64 & 0b10 ? 0b11'10'00'00 : 0b01'00'00'00);
        // clang-format on

        return _mm_shuffle_epi32(a, indices32);
    }
};

template<same_as_any<int32_t,uint32_t> T>
struct simd_shuffle<T, 4> {
    using reg_type = simd_reg_t<T, 4>;

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg_type operator()(reg_type a) const noexcept
    {
        constexpr auto indices = detail::make_shuffle_indices_imm<4, Indices...>();
        return _mm_shuffle_epi32(a, indices);
    }
};
#endif

#if defined(HI_HAS_AVX2)
template<>
struct simd_shuffle<double, 4> {
    using reg_type = simd_reg_t<double, 4>;

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg_type operator()(reg_type a) const noexcept
    {
        constexpr auto indices = detail::make_shuffle_indices_imm<4, Indices...>();
        return _mm256_permute4x64_pd(a, indices);
    }
};

template<same_as_any<int64_t,uint64_t> T>>
struct simd_shuffle<T, 4> {
    using reg_type = simd_reg_t<T, 4>;

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg_type operator()(reg_type a) const noexcept
    {
        constexpr auto indices = detail::make_shuffle_indices_imm<4, Indices...>();
        return _mm256_permute4x64_epi64(a, indices);
    }
};
#endif

#if defined(HI_HAS_AVX512F) and defined(HI_HAS_AVX512VL)
template<>
struct simd_shuffle<float, 8> {
    using reg_type = simd_reg_t<float, 8>;

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg_type operator()(reg_type a) const noexcept
    {
        auto indices = detail::make_shuffle_indices_xvar<uint32_t, 8, Indices...>();
        return _mm256_permutexvar_ps(indices, indices);
    }
};

template<same_as_any<int32_t,uint32_t> T>
struct simd_shuffle<T, 8> {
    using reg_type = simd_reg_t<T, 8>;

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg_type operator()(reg_type a) const noexcept
    {
        auto indices = detail::make_shuffle_indices_xvar<uint32_t, 8, Indices...>();
        return _mm256_permutexvar_epi32(indices, indices);
    }
};

template<same_as_any<int16_t,uint16_t,half> T>
struct simd_shuffle<T, 16> {
    using reg_type = simd_reg_t<T, 16>;

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg_type operator()(reg_type a) const noexcept
    {
        auto indices = detail::make_shuffle_indices_xvar<uint16_t, 16, Indices...>();
        return _mm256_permutexvar_epi16(indices, indices);
    }
};

template<same_as_any<int8_t,uint8_t> T>
struct simd_shuffle<T, 32> {
    using reg_type = simd_reg_t<T, 32>;

    template<int... Indices>
    [[nodiscard]] hi_force_inline reg_type operator()(reg_type a) const noexcept
    {
        auto indices = detail::make_shuffle_indices_xvar<uint8_t, 32, Indices...>();
        return _mm256_permutexvar_epi8(indices, indices);
    }
};
#endif


}}

