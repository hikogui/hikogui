// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../architecture.hpp"
#include <array>
#include <cstdint>
#include <type_traits>
#include <concepts>

#if defined(TT_X86_64_V2)
#include <emmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#endif

namespace tt {

using ri8x16 = std::array<int8_t, 16>;
using ru8x16 = std::array<uint8_t, 16>;
using ri16x8 = std::array<int16_t, 8>;
using ru16x8 = std::array<uint16_t, 8>;
using ri32x4 = std::array<int32_t, 4>;
using ru32x4 = std::array<uint32_t, 4>;
using rf32x4 = std::array<float, 4>;
using ri64x2 = std::array<int64_t, 2>;
using ru64x2 = std::array<uint64_t, 2>;
using rf64x2 = std::array<double, 2>;

#if defined(TT_X86_64_V2)

template<std::integral T, size_t N>
requires(sizeof(T) * N == 16) [[nodiscard]] inline __m128i to_m128i(std::array<T, N> const &rhs) noexcept
{
    return _mm_loadu_si128(reinterpret_cast<__m128i const *>(rhs.data()));
}

[[nodiscard]] inline __m128 to_m128(rf32x4 const &rhs) noexcept
{
    return _mm_loadu_ps(rhs.data());
}

[[nodiscard]] inline __m128d to_m128d(rf64x2 const &rhs) noexcept
{
    return _mm_loadu_pd(rhs.data());
}

#define X(x) \
    [[nodiscard]] inline x to_##x(__m128i rhs) noexcept \
    { \
        x r; \
        _mm_storeu_si128(reinterpret_cast<__m128i *>(r.data()), rhs); \
        return r; \
    }

X(ri8x16)
X(ru8x16)
X(ri16x8)
X(ru16x8)
X(ri32x4)
X(ru32x4)
X(ri64x2)
X(ru64x2)
#undef X

[[nodiscard]] inline rf32x4 to_rf32x4(__m128 rhs) noexcept
{
    rf32x4 r;
    _mm_storeu_ps(r.data(), rhs);
    return r;
}

[[nodiscard]] inline rf64x2 to_rf64x2(__m128d rhs) noexcept
{
    rf64x2 r;
    _mm_storeu_pd(r.data(), rhs);
    return r;
}

#endif

} // namespace tt
