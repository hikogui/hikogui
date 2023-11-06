// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file color/sRGB.hpp Color matrix and transfer functions for the sRGB color space.
 * @ingroup color
 */

module;
#include "../macros.hpp"

#include <cmath>
#include <array>
#include <algorithm>
#include <string_view>
#include <format>

export module hikogui_color_sRGB;
import hikogui_color_intf;
import hikogui_geometry;
import hikogui_utility;

hi_warning_push();
// C26426: Global initializer calls a non-constexpr function '...' (i.22).
// std::pow() is not constexpr and needed to fill in the gamma conversion tables.
hi_warning_ignore_msvc(26426);

export namespace hi { inline namespace v1 {

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
[[nodiscard]] float sRGB_linear_to_gamma(float u) noexcept
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
[[nodiscard]] float sRGB_gamma_to_linear(float u) noexcept
{
    if (u <= 0.04045) {
        return u / 12.92f;
    } else {
        return std::pow((u + 0.055f) / 1.055f, 2.4f);
    }
}

namespace detail {

[[nodiscard]] auto sRGB_linear16_to_gamma8_table_generator() noexcept
{
    std::array<uint8_t, 65536> r{};

    for (int i = 0; i != 65536; ++i) {
        r[i] = round_cast<uint8_t>(
            std::floor(std::clamp(sRGB_linear_to_gamma(float16(intrinsic, narrow_cast<uint16_t>(i))), 0.0f, 1.0f) * 255.0f));
    }

    return r;
}

[[nodiscard]] auto sRGB_gamma8_to_linear16_table_generator() noexcept
{
    std::array<float16, 256> r{};

    for (int i = 0; i != 256; ++i) {
        r[i] = static_cast<float16>(sRGB_gamma_to_linear(i / 255.0f));
    }

    return r;
}

auto sRGB_linear16_to_gamma8_table = sRGB_linear16_to_gamma8_table_generator();
auto sRGB_gamma8_to_linear16_table = sRGB_gamma8_to_linear16_table_generator();

} // namespace detail

/** sRGB linear float-16 to gamma transfer function.
 *
 * This function uses a lookup table for quick conversion.
 *
 * @ingroup color
 * @param u The linear color value, between 0.0 and 1.0.
 * @return The color value converted to the sRGB gamma corrected value between 0.0 and 1.0.
 */
[[nodiscard]] uint8_t sRGB_linear16_to_gamma8(float16 u) noexcept
{
    return detail::sRGB_linear16_to_gamma8_table[u.intrinsic()];
}

/** sRGB gamma to linear float-16 transfer function.
 *
 * This function uses a lookup table for quick conversion.
 *
 * @ingroup color
 * @param u The sRGB gamma corrected color value, between 0.0 and 1.0.
 * @return The color value converted to a linear color value between 0.0 and 1.0.
 */
[[nodiscard]] float16 sRGB_gamma8_to_linear16(uint8_t u) noexcept
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
[[nodiscard]] color color_from_sRGB(float r, float g, float b, float a) noexcept
{
    return color{sRGB_gamma_to_linear(r), sRGB_gamma_to_linear(g), sRGB_gamma_to_linear(b), a};
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
[[nodiscard]] color color_from_sRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
{
    return color_from_sRGB(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

[[nodiscard]] color color_from_sRGB(std::string_view str)
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

    auto packed = from_string<uint32_t>(tmp);

    uint8_t const r = truncate<uint8_t>(packed >> 24);
    uint8_t const g = truncate<uint8_t>(packed >> 16);
    uint8_t const b = truncate<uint8_t>(packed >> 8);
    uint8_t const a = truncate<uint8_t>(packed);
    return color_from_sRGB(r, g, b, a);
}

}} // namespace hi::v1

hi_warning_pop();
