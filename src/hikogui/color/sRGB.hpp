// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file color/sRGB.hpp Color matrix and transfer functions for the sRGB color space.
 * @ingroup color
 */

#pragma once

#include "../utility/module.hpp"
#include "../geometry/module.hpp"
#include "color.hpp"
#include <cmath>
#include <array>

hi_warning_push();
// C26426: Global initializer calls a non-constexpr function '...' (i.22).
// std::pow() is not constexpr and needed to fill in the gamma conversion tables.
hi_warning_ignore_msvc(26426);

namespace hi { inline namespace v1 {

/** Matrix to convert sRGB to XYZ.
 * @ingroup color
 */
constexpr matrix3 sRGB_to_XYZ =
    matrix3{0.41239080f, 0.35758434f, 0.18048079f, 0.21263901f, 0.71516868f, 0.07219232f, 0.01933082f, 0.11919478f, 0.95053215f};

/** Matrix to convert XYZ to sRGB.
 * @ingroup color
 */
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

/** sRGB linear to gamma transfer function.
 *
 * @ingroup color
 * @param u The linear color value, between 0.0 and 1.0.
 * @return The color value converted to the sRGB gamma corrected value between 0.0 and 1.0.
 */
[[nodiscard]] inline float sRGB_linear_to_gamma(float u) noexcept
{
    if (u <= 0.0031308) {
        return 12.92f * u;
    } else {
        return 1.055f * std::pow(u, 0.416f) - 0.055f;
    }
}

/** sRGB gamma to linear transfer function.
 *
 * @ingroup color
 * @param u The sRGB gamma corrected color value, between 0.0 and 1.0.
 * @return The color value converted to a linear color value between 0.0 and 1.0.
 */
[[nodiscard]] inline float sRGB_gamma_to_linear(float u) noexcept
{
    if (u <= 0.04045) {
        return u / 12.92f;
    } else {
        return std::pow((u + 0.055f) / 1.055f, 2.4f);
    }
}

namespace detail {

[[nodiscard]] inline auto sRGB_linear16_to_gamma8_table_generator() noexcept
{
    std::array<uint8_t, 65536> r{};

    for (int i = 0; i != 65536; ++i) {
        r[i] = narrow_cast<uint8_t>(
            std::floor(std::clamp(sRGB_linear_to_gamma(float16::from_uint16_t(narrow_cast<uint16_t>(i))), 0.0f, 1.0f) * 255.0f));
    }

    return r;
}

[[nodiscard]] inline auto sRGB_gamma8_to_linear16_table_generator() noexcept
{
    std::array<float16, 256> r{};

    for (int i = 0; i != 256; ++i) {
        r[i] = static_cast<float16>(sRGB_gamma_to_linear(i / 255.0f));
    }

    return r;
}

inline auto sRGB_linear16_to_gamma8_table = sRGB_linear16_to_gamma8_table_generator();
inline auto sRGB_gamma8_to_linear16_table = sRGB_gamma8_to_linear16_table_generator();

} // namespace detail

/** sRGB linear float-16 to gamma transfer function.
 *
 * This function uses a lookup table for quick conversion.
 *
 * @ingroup color
 * @param u The linear color value, between 0.0 and 1.0.
 * @return The color value converted to the sRGB gamma corrected value between 0.0 and 1.0.
 */
[[nodiscard]] inline uint8_t sRGB_linear16_to_gamma8(float16 u) noexcept
{
    return detail::sRGB_linear16_to_gamma8_table[u.get()];
}

/** sRGB gamma to linear float-16 transfer function.
 *
 * This function uses a lookup table for quick conversion.
 *
 * @ingroup color
 * @param u The sRGB gamma corrected color value, between 0.0 and 1.0.
 * @return The color value converted to a linear color value between 0.0 and 1.0.
 */
[[nodiscard]] inline float16 sRGB_gamma8_to_linear16(uint8_t u) noexcept
{
    return detail::sRGB_gamma8_to_linear16_table[u];
}

/** Convert gama corrected sRGB color to the linear color.
 *
 * @ingroup color
 * @param r The sRGB gamma corrected color value, between 0.0 and 1.0.
 * @param g The sRGB gamma corrected color value, between 0.0 and 1.0.
 * @param b The sRGB gamma corrected color value, between 0.0 and 1.0.
 * @param a Alpha value, between 0.0 and 1.0. not-premultiplied
 * @return A linear color.
 */
[[nodiscard]] inline color color_from_sRGB(
    std::floating_point auto r,
    std::floating_point auto g,
    std::floating_point auto b,
    std::floating_point auto a) noexcept
{
    return color{sRGB_gamma_to_linear(r), sRGB_gamma_to_linear(g), sRGB_gamma_to_linear(b), narrow_cast<float>(a)};
}

/** Convert gama corrected sRGB color to the linear color.
 *
 * @ingroup color
 * @param r The sRGB gamma corrected color value, between 0 and 255.
 * @param g The sRGB gamma corrected color value, between 0 and 255.
 * @param b The sRGB gamma corrected color value, between 0 and 255.
 * @param a Alpha value, between 0 and 255. not-premultiplied
 * @return A linear color.
 */
[[nodiscard]] inline color color_from_sRGB(
    std::integral auto r,
    std::integral auto g,
    std::integral auto b,
    std::integral auto a) noexcept
{
    hi_axiom_bounds(r, 0, 256);
    hi_axiom_bounds(g, 0, 256);
    hi_axiom_bounds(b, 0, 256);
    hi_axiom_bounds(a, 0, 256);
    return color_from_sRGB(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

[[nodiscard]] inline color color_from_sRGB(std::string_view str)
{
    auto tmp = std::string{str};

    if (tmp.starts_with("#")) {
        tmp = tmp.substr(1);
    }
    if (tmp.size() != 6 && tmp.size() != 8) {
        throw parse_error(std::format("Expecting 6 or 8 hex-digit sRGB color string, got {}.", str));
    }
    if (tmp.size() == 6) {
        tmp += "ff";
    }

    hilet packed = from_string<uint32_t>(tmp, 16);

    hilet r = truncate<uint8_t>(packed >> 24);
    hilet g = truncate<uint8_t>(packed >> 16);
    hilet b = truncate<uint8_t>(packed >> 8);
    hilet a = truncate<uint8_t>(packed);
    return color_from_sRGB(r, g, b, a);
}

}} // namespace hi::v1

hi_warning_pop();
