// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#if defined(HI_HAS_X86)
#include "cpu_id_x86.hpp"
#else
#include "cpu_id_generic.hpp"
#endif
#include <cstdint>
#include <bit>
#include <type_traits>
#include <array>

#ifdef HI_HAS_X86
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#endif

hi_export_module(hikogui.utility.half_to_float);

hi_export namespace hi { inline namespace v1 {

/** Convert half to float.
 *
 * This function is used
 */
[[nodiscard]] constexpr float half_to_float_generic(uint16_t u16) noexcept
{
    auto u32 = static_cast<uint32_t>(u16);

    auto sign = static_cast<int32_t>(u32);
    sign <<= 16;
    sign >>= 31;
    auto mantissa = u32;
    mantissa &= 0x3ff;
    mantissa <<= 22;
    auto exponent = static_cast<int32_t>(u32);
    exponent >>= 10;
    exponent &= 0x1f;
    exponent -= 15;

    if (exponent == -15) {
        if (mantissa == 0) {
            // Zero.
            exponent = -127;

        } else {
            // Denormal, translate to normal.
            auto shift = std::countl_zero(mantissa);
            mantissa <<= shift + 1;
            exponent -= shift;
        }

    } else if (exponent == 16) {
        // Infinite or NaN.
        exponent = 128;
    }

    mantissa >>= 9;
    exponent += 127;

    auto r = (sign << 31) | (exponent << 23) | mantissa;
    return std::bit_cast<float>(r);
}

namespace detail {

[[nodiscard]] consteval std::array<float, 65536> half_to_float_table_init() noexcept
{
    auto r = std::array<float, 65536>{};

    for (size_t i = 0; i != 65536; ++i) {
        r[i] = half_to_float_generic(static_cast<uint16_t>(i));
    }

    return r;
}

constexpr auto half_to_float_table = half_to_float_table_init();

}

#if HI_HAS_X86
hi_target("sse,sse2,f16c") [[nodiscard]] hi_inline std::array<float, 4> half_to_float_f16c(std::array<uint16_t, 4> v) noexcept
{
    auto const r = _mm_cvtph_ps(_mm_set1_epi64x(std::bit_cast<int64_t>(v)));

    auto r_ = std::array<float, 4>{};
    _mm_storeu_ps(r_.data(), r);
    return r_;
}
#endif

[[nodiscard]] constexpr std::array<float, 4> half_to_float(std::array<uint16_t, 4> v) noexcept
{
    if (not std::is_constant_evaluated()) {
#if HI_HAS_X86
        if (has_f16c()) {
            return half_to_float_f16c(v);
        }
#endif
    }

    auto r = std::array<float, 4>{};
    for (size_t i = 0; i != 4; ++i) {
        r[i] = detail::half_to_float_table[v[i]];
    }
    return r;
}

[[nodiscard]] constexpr float half_to_float(uint16_t v) noexcept
{
    if (not std::is_constant_evaluated()) {
#if HI_HAS_X86
        if (has_f16c()) {
            auto const v_ = std::array<uint16_t,4>{v, 0, 0, 0};
            auto const r = half_to_float_f16c(v_);
            return std::get<0>(r);
        }
#endif
    }

    return detail::half_to_float_table[v];
}


}}

