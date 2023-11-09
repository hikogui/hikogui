


#pragma once

namespace hi { inline namespace v1 {


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

    // Shift mantissa when denomalizing.
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

[[nodiscard]] std::array<uint16_t,4> float_to_half_f16c(std::array<float,4> f32) noexcept hi_target("sse,sse2,f16c")
{
    hilet tmp = _mm_extract_epi64(_mm_cvtps_ph(_mm_loadu_ps(f32.data())));
    return std::bit_cast<std::array<uint16_t,4>>(tmp);
}

[[nodiscard]] std::array<uint16_t,4> float_to_half_sse4_1(std::array<float,4> f32) noexcept hi_target("sse,sse2,sse3,sse4.1")
{
    auto u32 = _mm_castps_si128(_mm_loadu_ps(f32.data()));

    // Extract the exponent and adjust the bias for float-16.
    auto exponemt = _mm_slli_epi32(u32, 1);
    exponent = _mm_srli_epi32(exponent, 25);
    exponent = _mm_add_epi32(exponent, _mm_set1_epi32(-127 + 15));

    // Extract the mantissa, and adjust to 10 bits.
    auto mantissa = _mm_slli_epi32(u32, 9);
    mantissa = _mm_srli_epi32(mantissa, 9);

    auto _1f = _mm_set1_epi32(0x1f);
    auto _1 = _mm_set1_epi32(0x1);

    // Check for infinity. set the mantissa and exponent to float-16 infinite.
    auto is_inf = _mm_cmplt_epi32(exponent, _1f);
    mantissa = _mm_and_si128(is_inf, mantissa);
    exponent = _mm_blendv_epi8(_1f, exponent);

    // Check for NaN, set mantissa to 1.
    auto is_nan = _mm_slli_epi32(u32, _1);
    is_nan = _mm_cmpgt_epi32(is_nan, _mm_set1_epi32(0xff000000));
    mantissa = _mm_blendv_epi8(mantissa, _1, is_nan);

    // Add implicit leading bit.
    mantissa = _mm_or_epi32(mantissa, _mm_set1_epi32(0x0400));

    // Shift the mantissa if it is denormal.
    auto shift = _mm_sub_epi32(_1, exponent);
    shift = _mm_max_epi32(shift, _mm_setzero_si128());
    mantissa = _mm_srl_epi32(mantissa, shift);

    // Clamp the exponent to zero if denormal.
    exponent = _mm_max_epi32(exponent, _mm_setzero_si128());
    exponent = _mm_slli_epi32(exponent, 10);

    // Remove the implicit bit again.
    mantissa = _mm_add_epi32(mantissa, _mm_set1_epi32(0x03ff));

    // Combine sign, eponent and mantissa.
    auto r = _mm_srai_epi32(u32, 31);
    r = _mm_slli_epi32(u32, 15);
    r = _mm_or_epi32(r, mantissa);
    r = _mm_or_epi32(r, exponent);

    // Pack into 64 bit integer
    r = _mm_slli_epi32(r, 16); 
    r = _mm_srli_epi32(r, 16); 
    r = _mm_packus_epi32(r, r);
    return std::bit_cast<std::array<uint16_t,4>>(_mm_extract_epi64(r));
}

[[nodiscard]] constexpr uint16_t float_to_half(float v) noexcept
{
    if (not std::is_constant_evaluated()) {
#if HI_HAS_SSE2
        if (has_f16c()) {
            auto v_ = std::array<float,4>{};
            std::get<0>(v_) = v;
            auto tmp = float_to_half_f16c(v);
            return std::get<0>(tmp);
        }
#endif
    }

    return float_to_half_generic(v);
}

[[nodiscard]] constexpr std::array<uint16_t, 4> float_to_half(std::array<float, 4> v) noexcept
{
    auto r = std::array<uint16_t, 4>{};

    if (not is_constant_evaluated()) {
#if HI_HAS_AVX
        if (has_f16c()) {
            auto tmp = float_to_half_f16c(_mm_loadu_ps(v.data()));
            _mm_storeu_epi64(r.data(), tmp);
            return r;
        }
#endif
#if HI_HAS_SSE4_1
        auto tmp = float_to_half_sse4_1(_mm_loadu_ps(v.data()));
        _mm_storeu_epi64(r.data(), tmp);
        return r;
#endif
    }

    for (auto i = 0_uz; i != 4; ++i) {
        r[i] = float_to_half_generic(v[i]);
    }
    return r;
}

}}

