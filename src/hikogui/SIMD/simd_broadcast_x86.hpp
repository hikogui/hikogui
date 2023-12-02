

#pragma once


hi_export_module(hikogui.simd.simd_broadcast);

hi_export namespace hi { inline namespace v1 {

template<typename T, size_t N> simd_broadcast;

#if defined(HI_HAS_SSE)
template<>
struct simd_broadcast<float, 4> {
    using array_type = std::array<float, 4>;
    [[nodiscard]] array_type operator()(float const &rhs) noexcept
    {
        return simd_store<float, 4>{}(_mm_set1_ps(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<float, 4>{}(_mm_broadcastss_ps(simd_load<float, 4>{}(rhs)));
#else
        return simd_store<float, 4>{}(_mm_set1_ps(std::get<0>(rhs)));
#endif
    }
};
#endif

#if defined(HI_HAS_SSE2)
template<>
struct simd_broadcast<double, 2> {
    using array_type = std::array<double, 2>;
    [[nodiscard]] array_type operator()(double const &rhs) noexcept
    {
        return simd_store<double, 2>{}(_mm_set1_pd(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<double, 2>{}(_mm_broadcastsd_pd(simd_load<double, 2>{}(rhs)));
#else
        return simd_store<double, 2>{}(_mm_set1_pd(std::get<0>(rhs)));
#endif
    }
};

template<same_as_any<int64_t,uint64_t> T>
struct simd_broadcast<T, 2> {
    using array_type = std::array<T, 2>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 2>{}(_mm_set1_epi64x(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<T, 2>{}(_mm_broadcastq_epi64(simd_load<T, 2>{}(rhs)));
#else
        return simd_store<T, 2>{}(_mm_set1_epi64x(std::get<0>(rhs)));
#endif
    }
};

template<same_as_any<int32_t,uint32_t> T>
struct simd_broadcast<T, 4> {
    using array_type = std::array<T, 4>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 4>{}(_mm_set1_epi32(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<T, 4>{}(_mm_broadcastd_epi32(simd_load<T, 4>{}(rhs)));
#else
        return simd_store<T, 4>{}(_mm_set1_epi32(std::get<0>(rhs)));
#endif
    }
};

template<same_as_any<int16_t,uint16_t> T>
struct simd_broadcast<T, 8> {
    using array_type = std::array<T, 8>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 8>{}(_mm_set1_epi16(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<T, 8>{}(_mm_broadcastw_epi16(simd_load<T, 8>{}(rhs)));
#else
        return simd_store<T, 8>{}(_mm_set1_epi16(std::get<0>(rhs)));
#endif
    }
};

template<same_as_any<int8_t,uint8_t> T>
struct simd_broadcast<T, 16> {
    using array_type = std::array<T, 16>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 16>{}(_mm_set1_epi8(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<T, 16>{}(_mm_broadcastb_epi8(simd_load<T, 16>{}(rhs)));
#else
        return simd_store<T, 16>{}(_mm_set1_epi8(std::get<0>(rhs)));
#endif
    }
};
#endif

#if defined(HI_HAS_AVX)
template<>
struct simd_broadcast<double, 4> {
    using array_type = std::array<double, 4>;
    [[nodiscard]] array_type operator()(double const &rhs) noexcept
    {
        return simd_store<double, 4>{}(_mm256_set1_pd(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<double, 4>{}(_mm256_broadcastsd_pd(simd_load<double, 4>{}(rhs)));
#else
        return simd_store<double, 4>{}(_mm256_set1_pd(std::get<0>(rhs)));
#endif
    }
};

template<>
struct simd_broadcast<float, 8> {
    using array_type = std::array<float, 8>;
    [[nodiscard]] array_type operator()(float const &rhs) noexcept
    {
        return simd_store<float, 8>{}(_mm256_set1_ps(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<float, 8>{}(_mm256_broadcastss_ps(simd_load<float, 8>{}(rhs)));
#else
        return simd_store<float, 8>{}(_mm256_set1_ps(std::get<0>(rhs)));
#endif
    }
};

template<same_as_any<int64_t,uint64_t> T>
struct simd_broadcast<T, 4> {
    using array_type = std::array<T, 4>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 4>{}(_mm256_set1_epi64x(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<T, 4>{}(_mm256_broadcastq_epi64(simd_load<T, 4>{}(rhs)));
#else
        return simd_store<T, 4>{}(_mm256_set1_epi64x(std::get<0>(rhs)));
#endif
    }
};

template<same_as_any<int32_t,uint32_t> T>
struct simd_broadcast<T, 8> {
    using array_type = std::array<T, 8>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 8>{}(_mm256_set1_epi32(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<T, 8>{}(_mm256_broadcastd_epi32(simd_load<T, 8>{}(rhs)));
#else
        return simd_store<T, 8>{}(_mm256_set1_epi32(std::get<0>(rhs)));
#endif
    }
};

template<same_as_any<int16_t,uint16_t> T>
struct simd_broadcast<T, 16> {
    using array_type = std::array<T, 16>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 16>{}(_mm256_set1_epi16(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<T, 16>{}(_mm256_broadcastw_epi16(simd_load<T, 16>{}(rhs)));
#else
        return simd_store<T, 16>{}(_mm256_set1_epi16(std::get<0>(rhs)));
#endif
    }
};

template<same_as_any<int8_t,uint8_t> T>
struct simd_broadcast<T, 32> {
    using array_type = std::array<T, 32>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 32>{}(_mm256_set1_epi8(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
#if defined(HI_HAS_AVX2)
        return simd_store<T, 32>{}(_mm256_broadcastb_epi8(simd_load<T, 32>{}(rhs)));
#else
        return simd_store<T, 32>{}(_mm256_set1_epi8(std::get<0>(rhs)));
#endif
    }
};
#endif

#if defined(HI_HAS_AVX512F)
template<>
struct simd_broadcast<double, 8> {
    using array_type = std::array<double, 8>;
    [[nodiscard]] array_type operator()(double const &rhs) noexcept
    {
        return simd_store<double, 8>{}(_mm512_set1_pd(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
        return simd_store<double, 8>{}(_mm512_broadcastsd_pd(simd_load<double, 8>{}(rhs)));
    }
};

template<>
struct simd_broadcast<float, 16> {
    using array_type = std::array<float, 16>;
    [[nodiscard]] array_type operator()(float const &rhs) noexcept
    {
        return simd_store<float, 16>{}(_mm512_set1_ps(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
        return simd_store<float, 16>{}(_mm512_broadcastss_ps(simd_load<float, 16>{}(rhs)));
    }
};

template<same_as_any<int64_t,uint64_t> T>
struct simd_broadcast<T, 8> {
    using array_type = std::array<T, 8>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 8>{}(_mm512_set1_epi64(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
        return simd_store<T, 8>{}(_mm512_broadcastq_epi64(simd_load<T, 8>{}(rhs)));
    }
};

template<same_as_any<int32_t,uint32_t> T>
struct simd_broadcast<T, 16> {
    using array_type = std::array<T, 16>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 16>{}(_mm512_set1_epi32(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
        return simd_store<T, 16>{}(_mm512_broadcastd_epi32(simd_load<T, 16>{}(rhs)));
    }
};

template<same_as_any<int16_t,uint16_t> T>
struct simd_broadcast<T, 32> {
    using array_type = std::array<T, 32>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 32>{}(_mm512_set1_epi16(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
        return simd_store<T, 32>{}(_mm512_broadcastw_epi16(simd_load<T, 32>{}(rhs)));
    }
};

template<same_as_any<int8_t,uint8_t> T>
struct simd_broadcast<T, 64> {
    using array_type = std::array<T, 64>;
    [[nodiscard]] array_type operator()(T const &rhs) noexcept
    {
        return simd_store<T, 64>{}(_mm512_set1_epi8(rhs));
    }
    [[nodiscard]] array_type operator()(array_type const &rhs) noexcept
    {
        return simd_store<T, 64>{}(_mm512_broadcastb_epi8(simd_load<T, 64>{}(rhs)));
    }
};
#endif



}}
