


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

hi_export_module(hikogui.utility.float_to_half);

hi_export namespace hi { inline namespace v1 {

[[nodiscard]] constexpr static uint16_t float_to_half_generic(float a) noexcept
{
    auto u32 = std::bit_cast<uint32_t>(a);

    // Extract exponent.
    auto exponent = static_cast<int16_t>(static_cast<uint8_t>((u32 << 1) >> 24)) - 127 + 15;

    // Extract the 24-bit mantissa.
    auto mantissa = (u32 << 9) >> (9 + 24 - 11);

    auto const is_inf = exponent >= 0x1f;
    if (is_inf) {
        exponent = 0x1f;
        mantissa = 0;
    }

    // Handle NaN.
    auto const is_nan = (u32 << 1) > 0xff000000;
    if (is_nan) {
        mantissa = 1;
    }

    // Add implicit leading bit.
    mantissa |= 0x0400;

    // Shift mantissa when denormalizing.
    auto shift = 1 - exponent;
    if (shift < 0) {
        shift = 0;
    }
    if (shift > 31) {
        shift = 31;
    }
    mantissa >>= shift;

    // Adjust exponent for denormals and zero.
    if (exponent < 0) {
        exponent = 0;
    }

    // Remove implicit leading bit.
    mantissa &= 0x03ff;

    // Extract sign.
    auto r = static_cast<uint16_t>((static_cast<int32_t>(u32) >> 31) << 15);
    r |= exponent << 10;
    r |= mantissa;
    return r;
}

[[nodiscard]] constexpr std::array<uint16_t,4> float_to_half_generic(std::array<float,4> a) noexcept
{
    auto r = std::array<uint16_t,4>{};
    for (size_t i = 0; i != 4; ++i) {
        r[i] = float_to_half_generic(a[i]);
    }
    return r;
}

#if HI_HAS_X86
hi_target("sse,sse2,f16c")
[[nodiscard]] inline std::array<uint16_t,4> float_to_half_f16c(std::array<float,4> a) noexcept
{
    auto const a_ = _mm_loadu_ps(a.data());
    auto const r = _mm_cvtps_ph(a_, _MM_FROUND_CUR_DIRECTION);
    return std::bit_cast<std::array<uint16_t,4>>(_mm_cvtsi128_si64(r));
}
#endif


#if HI_HAS_X86
hi_target("sse,sse2")
[[nodiscard]] inline std::array<uint16_t,4> float_to_half_sse2(std::array<float,4> a) noexcept
{
    auto r = _mm_castps_si128(_mm_loadu_ps(a.data()));

    // Extract the sign into bit 15.
    auto const sign = _mm_slli_epi32(_mm_srli_epi32(r, 31), 15);

    // Strip off the sign.
    r = _mm_srli_epi32(_mm_slli_epi32(r, 1), 1);

    auto const is_nan = _mm_cmpgt_epi32(r, _mm_set1_epi32(0x7f80'0000));
    if (_mm_movemask_epi8(is_nan) != 0) {
        return float_to_half_generic(a);
    }

    auto const is_zero = _mm_cmpeq_epi32(r, _mm_setzero_si128());

    // Adjust exponent.
    r = _mm_sub_epi32(r, _mm_set1_epi32(112 * 0x0040'0000));

    // If after adjustment the exponent is zero or less, then it is a denormal.
    auto const is_denorm = _mm_andnot_si128(is_zero, _mm_cmpgt_epi32(_mm_setzero_si128(), r));
    if (_mm_movemask_epi8(is_denorm) != 0) {
        return float_to_half_generic(a);
    }

    // Make sure the value is zero if the original was zero.
    r = _mm_andnot_si128(is_zero, r);

    // If after adjustment the exponent is greater or equal to 0x1f then the value is infinite.
    // Then make the value not go over infinite.
    auto const inf_value = _mm_set1_epi32(0x0f80'0000);
    auto const is_inf = _mm_cmpgt_epi32(r, inf_value);
    r = _mm_andnot_si128(is_inf, r);
    r = _mm_or_si128(r, _mm_and_si128(is_inf, inf_value));

    // Shift to fit inside 16-bits.
    r = _mm_srli_epi32(r, 13);

    // Add the sign back in.
    r = _mm_or_si128(r, sign);

    // Pack 16 bit values in lower half.
    r = _mm_shufflelo_epi16(r, 0b11'11'10'00);
    r = _mm_shufflehi_epi16(r, 0b11'11'10'00);
    r = _mm_shuffle_epi32(r, 0b11'11'10'00);
    return std::bit_cast<std::array<uint16_t,4>>(_mm_cvtsi128_si64(r));
}
#endif

[[nodiscard]] constexpr uint16_t float_to_half(float v) noexcept
{
    if (not std::is_constant_evaluated()) {
#if HI_HAS_X86
        if (has_f16c()) {
            auto v_ = std::array<float,4>{v, v, v, v};
            auto tmp = float_to_half_f16c(v_);
            return std::get<0>(tmp);
        }
#endif
    }

    return float_to_half_generic(v);
}

[[nodiscard]] constexpr std::array<uint16_t, 4> float_to_half(std::array<float, 4> v) noexcept
{
    auto r = std::array<uint16_t, 4>{};

    if (not std::is_constant_evaluated()) {
#if HI_HAS_X86
        if (has_f16c()) {
            return float_to_half_f16c(v);
        }
        if (has_sse2()) {
            return float_to_half_sse2(v);
        }
#endif
    }

    for (size_t i = 0; i != 4; ++i) {
        r[i] = float_to_half_generic(v[i]);
    }
    return r;
}

}}

