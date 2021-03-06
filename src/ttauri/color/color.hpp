// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/numeric_array.hpp"

namespace tt {

/** This is a RGBA floating point color.
 * The color can be converted between different color spaces using the matrix-class.
 *
 * But in most cases in the application and ttauri library this color would be in
 * the esRGBA color space. This color space is compatible with the sRGB standard
 * IEC 61966-2-1:1999.
 *
 * esRGB details:
 * - the ITU-R BT.709 color primaries.
 * - A linear transfer function (unlike sRGB).
 * - R=0.0, G=0.0, B=0.0: Black
 * - R=1.0, G=1.0, B=1.0: White D65 at 80 nits (80 cd/m^2).
 * - RGB values above 1.0 are allowed for HDR (high dynamic range)
 * - RGB values below 0.0 are allowed for WCG (wide color gamut)
 *
 * esRGBA details:
 * - Includes an alpha value
 * - Alpha values are linear and must be between 0.0 through 1.0.
 * - A=0.0 fully transparent
 * - A=1.0 fully opaque
 * - RGB values are NOT pre-multiplied with the alpha value.
 *
 * This color format is inspired by scRGB, however scRGB only describes
 * a 12- or 16-bit integer per component encoding of RGB values between
 * -0.5 and 7.5.
 *
 * It is also inspired by Apple's extended sRGB format.
 */
class color {
public:
    constexpr color(color const &) noexcept = default;
    constexpr color(color &&) noexcept = default;
    constexpr color &operator=(color const &) noexcept = default;
    constexpr color &operator=(color &&) noexcept = default;

    [[nodiscard]] constexpr explicit color(f32x4 other) noexcept : _v(std::move(other))
    {
        tt_axiom(is_valid());
    }
    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr color() noexcept : _v(0.0, 0.0, 0.0, 1.0) {}
    [[nodiscard]] constexpr color(float r, float g, float b, float a = 1.0) noexcept : _v(r, g, b, a) {}

    [[nodiscard]] constexpr float &r() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float &g() noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float &b() noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float &a() noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr float const &r() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float const &g() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float const &b() const noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float const &a() const noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return _v.w() >= 0.0 && _v.w() <= 1.0;
    }

    [[nodiscard]] constexpr friend bool operator==(color const &lhs, color const &rhs) noexcept
    {
        return lhs._v == rhs._v;
    }

    [[nodiscard]] constexpr friend color operator*(color const &lhs, color const &rhs) noexcept
    {
        return color{lhs._v * rhs._v};
    }

    [[nodiscard]] constexpr friend color composit(color const &lhs, color const &rhs) noexcept
    {
        return color{composit(lhs._v, rhs._v)};
    }

private:
    f32x4 _v;
};

} // namespace tt
