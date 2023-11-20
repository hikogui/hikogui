


#pragma once

#include "cpu_id_x86.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <bit>
#include <type_traits>

#ifdef HI_HAS_X86
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#endif

hi_export_module(hikogui.utility.float_to_half);

hi_export namespace hi { inline namespace v1 {

[[nodiscard]] constexpr uint16_t float_to_half_generic(float f32) noexcept
{
    auto u32 = std::bit_cast<uint32_t>(f32);

    // Extract exponent.
    auto exponent = static_cast<int16_t>(static_cast<uint8_t>((u32 << 1) >> 24)) - 127 + 15;

    // Extract the 24-bit mantissa.
    auto mantissa = (u32 << 9) >> (9 + 24 - 11);

    auto is_inf = exponent >= 0x1f;
    if (is_inf) {
        exponent = 0x1f;
        mantissa = 0;
    }

    // Handle NaN.
    auto is_nan = (u32 << 1) > 0xff000000;
    if (is_nan) {
        mantissa = 1;
    }

    // Add implicit leading bit.
    mantissa |= 0x0400;

    // Shift mantissa when denormalizing.
    auto shift = 1 - exponent;
    shift = shift < 0 ? 0 : shift;
    shift = shift > 31 ? 31 : shift;
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

#if HI_HAS_X86
hi_target("sse,sse4.1,f16c")
[[nodiscard]] hi_inline std::array<uint16_t,4> float_to_half_f16c(std::array<float,4> f32) noexcept
{
    return std::bit_cast<std::array<uint16_t, 4>>(
        _mm_extract_epi64(_mm_cvtps_ph(_mm_loadu_ps(f32.data()), _MM_FROUND_TO_ZERO), 0));
}
#endif

#if HI_HAS_X86
hi_target("sse,sse2,f16c")
[[nodiscard]] hi_inline uint16_t float_to_half_f16c(float f32) noexcept
{
    return static_cast<uint16_t>(_mm_extract_epi16(_mm_cvtps_ph(_mm_set1_ps(f32), _MM_FROUND_TO_ZERO), 0));
}
#endif

#if HI_HAS_X86
hi_target("sse,sse2,sse4.1")
[[nodiscard]] hi_inline std::array<uint16_t,4> float_to_half_sse4_1(std::array<float,4> f32) noexcept
{
    auto u32 = _mm_castps_si128(_mm_loadu_ps(f32.data()));

    // Extract the sign into the lsb.
    hilet sign = _mm_srli_epi32(u32, 31);

    // Strip off the sign.
    u32 = _mm_srli_epi32(_mm_slli_epi32(u32, 1), 1);

    // Extract the exponent, and adjust for float-16 bias.
    auto exponent = _mm_srli_epi32(u32, 23);
    exponent = _mm_add_epi32(exponent, _mm_set1_epi32(-127 + 15));

    // Extract the mantissa, and adjust for float-16.
    auto mantissa = _mm_slli_epi32(u32, 9);
    mantissa = _mm_srli_epi32(mantissa, 9 + 13);

    // Check for infinity. When the exponent >= 0x1f.
    // Set mantissa to zero if infinite.
    hilet _1f = _mm_set1_epi32(0x1f);
    hilet is_inf = _mm_cmplt_epi32(_1f, exponent);
    mantissa = _mm_andnot_si128(is_inf, mantissa);

    // On NaN, set mantissa to 1.
    hilet is_nan = _mm_cmpgt_epi32(u32, _mm_set1_epi32(0x7f80'0000));
    hilet _1 = _mm_set1_epi32(0x1);
    mantissa = _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(mantissa), _mm_castsi128_ps(_1), _mm_castsi128_ps(is_nan)));

    // Add implicit leading bit.
    hilet _0400 = _mm_set1_epi32(0x0400);
    mantissa = _mm_or_si128(mantissa, _0400);

    // Shift the mantissa if it is denormal.
    auto shift = _mm_sub_epi32(_1, exponent);
    shift = _mm_max_epi32(shift, _mm_setzero_si128());
    if (not _mm_testz_si128(shift, shift)) {
        // Emulate vector right-shifts.
        hilet shift_ = _mm_castsi128_ps(shift);
        auto shift0 = _mm_castps_si128(_mm_insert_ps(shift_, shift_, 0b00'00'1110));
        auto shift1 = _mm_castps_si128(_mm_insert_ps(shift_, shift_, 0b01'00'1110));
        auto shift2 = _mm_castps_si128(_mm_insert_ps(shift_, shift_, 0b10'00'1110));
        auto shift3 = _mm_castps_si128(_mm_insert_ps(shift_, shift_, 0b11'00'1110));
        auto mantissa0 = _mm_castsi128_ps(_mm_srl_epi32(mantissa, shift0));
        auto mantissa1 = _mm_castsi128_ps(_mm_srl_epi32(mantissa, shift1));
        auto mantissa2 = _mm_castsi128_ps(_mm_srl_epi32(mantissa, shift2));
        auto mantissa3 = _mm_castsi128_ps(_mm_srl_epi32(mantissa, shift3));
        mantissa = _mm_castps_si128(_mm_insert_ps(_mm_castsi128_ps(mantissa), mantissa0, 0b00'00'0000));
        mantissa = _mm_castps_si128(_mm_insert_ps(_mm_castsi128_ps(mantissa), mantissa1, 0b01'01'0000));
        mantissa = _mm_castps_si128(_mm_insert_ps(_mm_castsi128_ps(mantissa), mantissa2, 0b10'10'0000));
        mantissa = _mm_castps_si128(_mm_insert_ps(_mm_castsi128_ps(mantissa), mantissa3, 0b11'11'0000));
    }

    // Remove the implicit bit again, if it didn't move.
    mantissa = _mm_andnot_si128(_0400, mantissa);

    // Clamp the exponent between 0 (denormal) and 0x1f (infinite,NaN).
    exponent = _mm_min_epi32(exponent, _1f);
    exponent = _mm_max_epi32(exponent, _mm_setzero_si128());

    // Combine the sign and exponent onto the mantissa.
    mantissa = _mm_or_si128(mantissa, _mm_slli_epi32(sign, 15));
    mantissa = _mm_or_si128(mantissa, _mm_slli_epi32(exponent, 10));

    // Convert each uint32_t to uint16_t and pack them together into a single uint64_t.
    mantissa = _mm_packus_epi32(mantissa, mantissa);
    return std::bit_cast<std::array<uint16_t,4>>(_mm_extract_epi64(mantissa, 0));
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
        if (has_sse4_1()) {
            return float_to_half_sse4_1(v);
        }
#endif
    }

    for (auto i = 0_uz; i != 4; ++i) {
        r[i] = float_to_half_generic(v[i]);
    }
    return r;
}

}}

