// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <array>
#include <emmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>

namespace tt {

using i8x16_raw = std::array<int8_t, 16>;

[[nodiscard]] inline i8x16_raw to_i8x16_raw(__m128i const &rhs) noexcept
{
    i8x16_raw r;
    _mm_storeu_si128(reinterpret_cast<__m128i *>(r.data()), rhs);
    return r;
}

[[nodiscard]] inline __m128i to_m128i(i8x16_raw const &rhs) noexcept
{
    return _mm_loadu_si128(reinterpret_cast<__m128i const *>(rhs.data()));
}

/** Shuffle the bytes in the SSE register
 */
[[nodiscard]] inline i8x16_raw i8x16_x64v2_undefined() noexcept
{
    return to_i8x16_raw(_mm_undefined_si128());
}

/** Or bits in lhs with bits in rhs.
 */
[[nodiscard]] inline i8x16_raw i8x16_x64v2_or(i8x16_raw const &lhs, i8x16_raw const &rhs) noexcept
{
    return to_i8x16_raw(_mm_or_si128(to_m128i(lhs), to_m128i(rhs)));
}

} // namespace tt
