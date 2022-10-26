// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../float16.hpp"
#include "../geometry/matrix.hpp"
#include "../codec/base_n.hpp"
#include "color.hpp"
#include <cmath>
#include <array>

hi_warning_push();
// C26426: Global initializer calls a non-constexpr function '...' (i.22).
// std::pow() is not constexpr and needed to fill in the gamma conversion tables.
hi_warning_ignore_msvc(26426);

namespace hi::inline v1 {

constexpr matrix3 sRGB_to_XYZ =
    matrix3{0.41239080f, 0.35758434f, 0.18048079f, 0.21263901f, 0.71516868f, 0.07219232f, 0.01933082f, 0.11919478f, 0.95053215f};

constexpr matrix3 XYZ_to_sRGB = matrix3{
    3.24096994f,
    -1.53738318f,
    -0.49861076f,
    -0.96924364f,
    1.87596750f,
    0.04155506f,
    0.05563008f,
    -0.20397696f,
    1.05697151f};

[[nodiscard]] inline float sRGB_linear_to_gamma(float u) noexcept
{
    if (u <= 0.0031308) {
        return 12.92f * u;
    } else {
        return 1.055f * std::pow(u, 0.416f) - 0.055f;
    }
}

[[nodiscard]] inline float sRGB_gamma_to_linear(float u) noexcept
{
    if (u <= 0.04045) {
        return u / 12.92f;
    } else {
        return std::pow((u + 0.055f) / 1.055f, 2.4f);
    }
}

[[nodiscard]] inline auto sRGB_linear16_to_gamma8_table_generator() noexcept
{
    std::array<uint8_t, 65536> r{};

    for (int i = 0; i != 65536; ++i) {
        r[i] = truncate<uint8_t>(
            std::clamp(sRGB_linear_to_gamma(float16::from_uint16_t(narrow_cast<uint16_t>(i))), 0.0f, 1.0f) * 255.0f);
    }

    return r;
}

inline auto sRGB_linear16_to_gamma8_table = sRGB_linear16_to_gamma8_table_generator();

[[nodiscard]] inline uint8_t sRGB_linear16_to_gamma8(float16 u) noexcept
{
    return sRGB_linear16_to_gamma8_table[u.get()];
}

[[nodiscard]] inline auto sRGB_gamma8_to_linear16_table_generator() noexcept
{
    std::array<float16, 256> r{};

    for (int i = 0; i != 256; ++i) {
        r[i] = static_cast<float16>(sRGB_gamma_to_linear(i / 255.0f));
    }

    return r;
}

inline auto sRGB_gamma8_to_linear16_table = sRGB_gamma8_to_linear16_table_generator();

[[nodiscard]] inline float16 sRGB_gamma8_to_linear16(uint8_t u) noexcept
{
    return sRGB_gamma8_to_linear16_table[u];
}

[[nodiscard]] inline color color_from_sRGB(float r, float g, float b, float a) noexcept
{
    return color{
        sRGB_gamma_to_linear(narrow_cast<float>(r)),
        sRGB_gamma_to_linear(narrow_cast<float>(g)),
        sRGB_gamma_to_linear(narrow_cast<float>(b)),
        a};
}

[[nodiscard]] inline color color_from_sRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
{
    return color_from_sRGB(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

[[nodiscard]] inline color color_from_sRGB(std::string_view str)
{
    auto tmp = std::string{str};

    if (tmp.starts_with("#")) {
        tmp = tmp.substr(1);
    }
    if (ssize(tmp) != 6 && ssize(tmp) != 8) {
        throw parse_error(std::format("Expecting 6 or 8 hex-digit sRGB color string, got {}.", str));
    }
    if (ssize(tmp) == 6) {
        tmp += "ff";
    }

    uint8_t const r = (base16::int_from_char<uint8_t>(tmp[0]) << 4) | base16::int_from_char<uint8_t>(tmp[1]);
    uint8_t const g = (base16::int_from_char<uint8_t>(tmp[2]) << 4) | base16::int_from_char<uint8_t>(tmp[3]);
    uint8_t const b = (base16::int_from_char<uint8_t>(tmp[4]) << 4) | base16::int_from_char<uint8_t>(tmp[5]);
    uint8_t const a = (base16::int_from_char<uint8_t>(tmp[6]) << 4) | base16::int_from_char<uint8_t>(tmp[7]);
    return color_from_sRGB(r, g, b, a);
}

} // namespace hi::inline v1

hi_warning_pop();
