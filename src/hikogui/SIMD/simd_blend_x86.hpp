



hi_export_module(hikogui.simd.simd_blend);

namespace hi { inline namespace v1 {

template<typename T, size_t N>
struct simd_blend;

#if defined(HI_HAS_SSE4_1)
template<>
struct simd_blend<float, 4> {
    using reg_type = simd_reg_t<float, 4>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111);
        return _mm_blend_ps(a, b, Mask);
    }
};

#elif defined(HI_HAS_SSE)
template<>
struct simd_blend<float, 4> {
    using reg_type = simd_reg_t<float, 4>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(array_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111);

        // clang-format off
        constexpr auto indices =
            (Mask & 0b0001 ? 0b00'00'00'01 : 0b00'00'00'00) |
            (Mask & 0b0010 ? 0b00'00'11'00 : 0b00'00'10'00) |
            (Mask & 0b0100 ? 0b00'01'00'00 : 0b00'00'00'00) |
            (Mask & 0b1000 ? 0b11'00'00'00 : 0b10'00'00'00);
        // clang-format on

        hilet lo = _mm_unpacklo_ps(a, b);
        hilet hi = _mm_unpacklo_ps(a, b);
        return _mm_shuffle_ps(lo, hi, indices);
    }
};
#endif

#if defined(HI_HAS_SSE4_1)
template<same_as_any<int32_t,uint32_t> T>
struct simd_blend<T, 4> {
    using reg_type = simd_reg_t<T, 4>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111);

        hilet a_ = _mm_castsi128_ps(a);
        hilet b_ = _mm_castsi128_ps(b);
        hilet r = _mm_blend_ps(a_, b_, Mask);
        return _mm_castps_si128(r);
    }
};

#elif defined(HI_HAS_SSE2)
template<same_as_any<int32_t,uint32_t> T>
struct simd_blend<T, 4> {
    using reg_type = simd_reg_t<T, 4>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111);

        // clang-format off
        constexpr auto indices =
            (Mask & 0b0001 ? 0b00'00'00'01 : 0b00'00'00'00) |
            (Mask & 0b0010 ? 0b00'00'11'00 : 0b00'00'10'00) |
            (Mask & 0b0100 ? 0b00'01'00'00 : 0b00'00'00'00) |
            (Mask & 0b1000 ? 0b11'00'00'00 : 0b10'00'00'00);
        // clang-format on

        hilet a_ = _mm_castsi128_ps(a);
        hilet b_ = _mm_castsi128_ps(b);
        hilet lo = _mm_unpacklo_ps(a_, b_);
        hilet hi = _mm_unpacklo_ps(a_, b_);
        hilet r = _mm_shuffle_ps(lo, hi, indices);
        return _mm_castps_si128(r);
    }
};
#endif

#if defined(HI_HAS_SSE4_1)
template<same_as_any<int64_t,uint64_t> T>
struct simd_blend<T, 2> {
    using reg_type = simd_reg_t<T, 2>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b11);

        // clang-format off
        constexpr auto mask =
            (Mask & 0b01 ? 0b0011 : 0b0000) |
            (Mask & 0b10 ? 0b1100 : 0b0000);
        // clang-format on

        hilet a_ = _mm_castsi128_ps(a);
        hilet b_ = _mm_castsi128_ps(b);
        hilet r = _mm_blend_ps(a_, b_, mask);
        return _mm_castps_si128(r);
    }
};

#elif defined(HI_HAS_SSE2)
template<same_as_any<int64_t,uint64_t> T>
struct simd_blend<T, 2> {
    using reg_type = simd_reg_t<T, 2>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b11);

        // clang-format off
        constexpr auto indices =
            (Mask & 0b0001 ? 0b00'00'11'01 : 0b00'00'10'00) |
            (Mask & 0b0100 ? 0b11'01'00'00 : 0b10'00'00'00);
        // clang-format on

        hilet a_ = _mm_castsi128_ps(a);
        hilet b_ = _mm_castsi128_ps(b);
        hilet lo = _mm_unpacklo_ps(a_, b_);
        hilet hi = _mm_unpacklo_ps(a_, b_);
        hilet r = _mm_shuffle_ps(lo, hi, indices);
        return _mm_castps_si128(r);
    }
};
#endif

#if defined(HI_HAS_SSE4_1)
template<same_as_any<int16_t,uint16_t,half> T>
struct simd_blend<T, 8> {
    using reg_type = simd_reg_t<T, 8>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111'1111);

        return _mm_blend_epi16(a, b, Mask);
    }
};
#endif

#if defined(HI_HAS_AVX)
struct simd_blend<float, 8> {
    using reg_type = simd_reg_t<float, 8>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111'1111);

        return _mm256_blend_ps(a, b, Mask);
    }
};

struct simd_blend<double, 4> {
    using reg_type = simd_reg_t<double, 4>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111);

        return _mm256_blend_pd(a, b, Mask);
    }
};
#endif

#if defined(HI_HAS_AVX2)
template<same_as_any<int16_t,uint16_t,half> T>
struct simd_blend<T, 16> {
    using reg_type = simd_reg_t<T, 16>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111'1111'1111'1111);

        constexpr auto mask0 = Mask & 0b1111'1111;
        constexpr auto mask1 = Mask >> 8;

        hilet lo = _mm256_blend_epi16(a, b, mask0);
        hilet hi = _mm256_blend_epi16(a, b, mask1);
        return _mm256_blend_epi32(lo, hi, 0b1100);
    }
};

template<same_as_any<int32_t,uint32_t> T>
struct simd_blend<T, 8> {
    using reg_type = simd_reg_t<T, 8>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111'1111);

        return _mm256_blend_epi32(a, b, Mask);
    }
};

template<same_as_any<int64_t,uint64_t> T>
struct simd_blend<T, 4> {
    using reg_type = simd_reg_t<T, 4>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111);

        // clang-format off
        constexpr auto mask =
            (Mask & 0b0001 ? 0b0000'0011 : 0b0000'0000) |
            (Mask & 0b0010 ? 0b0000'1100 : 0b0000'0000) |
            (Mask & 0b0100 ? 0b0011'0000 : 0b0000'0000) |
            (Mask & 0b1000 ? 0b1100'0000 : 0b0000'0000);
        // clang-format on

        return _mm256_blend_epi32(a, b, mask);
    }
};
#endif

#if defined(HI_HAS_AVX512F)
struct simd_blend<float, 16> {
    using reg_type = simd_reg_t<float, 16>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111'1111'1111'1111);

        return _mm512_mask_blend_ps(Mask, a, b);
    }
};

struct simd_blend<double, 8> {
    using reg_type = simd_reg_t<double, 8>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111'1111);

        return _mm512_mask_blend_pd(Mask, a, b);
    }
};

template<same_as_any<int32_t,uint32_t> T>
struct simd_blend<T, 16> {
    using reg_type = simd_reg_t<T, 16>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111'1111'1111'1111);

        return _mm512_mask_blend_epi32(Mask, a, b);
    }
};

template<same_as_any<int64_t,uint64_t> T>
struct simd_blend<T, 8> {
    using reg_type = simd_reg_t<T, 8>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0b1111'1111);

        return _mm512_mask_blend_epi64(Mask, a, b);
    }
};
#endif

#if defined(HI_HAS_AVX512BW)
template<same_as_any<int16_t,uint16_t,half> T>
struct simd_blend<T, 32> {
    using reg_type = simd_reg_t<T, 32>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        static_assert(Mask <= 0xffff'ffff);

        return _mm512_mask_blend_epi16(Mask, a, b);
    }
};

template<same_as_any<int8_t,uint8_t> T>
struct simd_blend<T, 64> {
    using reg_type = simd_reg_t<T, 64>;

    template<size_t Mask>
    [[nodiscard]] hi_force_inline reg_type blend(reg_type a, reg_type b) const noexcept
    {
        hilet r = _mm512_mask_blend_epi8(Mask, a, b);
    }
};
#endif

}}

