

#pragma once

namespace hi { inline namespace v1 {

namespace detail {

/** Convert half to float.
 *
 * This function is used
 */
[[nodiscard]] consteval float half_to_float_generic(uint16_t u16) noexcept
{
    auto u32 = static_cast<uint32_t>(u16);

    auto sign = static_cast<int32_t>(u32) >> 31;
    auto mantissa = static_cast<int32_t>(u32 & 0x3ff) << 22;
    auto exponent = static_cast<int32_t>(static_cast<uint8_t>(u32 >> 10)) - 15;

    if (exponent == -15)
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
        exponent = 127;
    }

    mantissa >>= 9;
    exponent += 127;

    auto r = (sign << 31) | (exponent << 23) | mantissa;
    return std::bit_cast<float>(r);
}

[[nodiscard]] consteval std::array<float, 65536> half_to_float_table_init() noexcept
{
    auto r = std::array<float, 65536>{};

    for (auto i = 0_uz; i != 65536; ++i) {
        r[i] = half_to_float_generic(static_cast<uint16_t>(i));
    }

    return r;
}

constexpr auto half_to_float_table = half_to_float_table_init();

}

[[nodiscard]] constexpr std::array<float, 4> half_to_float(std::array<uint16_t, 4> v) noexcept
{
    auto r = std::array<float, 4>{};

    if (not std::is_constant_evaluated()) {
#if HI_HAS_AVX
        if (has_f16c()) {
            hilet t1 = std::bit_cast<int64_t>(v);
            hilet t2 = _mm_set1_epi64x(t1);
            hilet t3 = _mm_cvtph_ps(t2);
            _mm_storeu_ps(t3, r.data());
            return r;
        }
#endif
#if HI_HAS_AVX2
        hilet t1 = std::bit_cast<int64_t>(v)
        hilet t2 = _mm_set1_epi64x(t1);
        hilet t3 = _mm_cvtepu16_epi32(t2);
        hilet t4 = _mm_i32gather_ps(half_to_float_table.data(), t3, sizeof(float));
        _mm_storeu_ps(t3, r.data());
        return r;
#endif
    }

    for (auto i = 0_uz; i != 4; ++i) {
        r[i] = detail::half_to_float_table[v[i]];
    }
    return r;
}

[[nodiscard]] constexpr float half_to_float(uint16_t v) noexcept
{
    if (not std::is_constant_evaluated()) {
#if HI_HAS_AVX
        if (has_f16c()) {
            hilet tmp = _mm_cvtph_ps(_mm_set1_epi16(static_cast<short>(v)));
            return _mm_extract_ps(tmp, 0);
        }
#endif
    }

    return detail::half_to_float_table[v];
}


}}

