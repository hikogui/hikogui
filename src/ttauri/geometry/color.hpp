// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../numeric_array.hpp"

namespace tt::geo {

/** A esRGBA (extended-sRGB with alpha) color.
 * This color compatible with the sRGB standard IEC 61966-2-1:1999
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

    [[nodiscard]] constexpr explicit color(f32x4 other) noexcept : v(std::move(other)) { tt_axiom(v.w() >= 0.0 && v.w() <= 1.0); }
    [[nodiscard]] constexpr explicit operator f32x4 () const noexcept { return v; }

    [[nodiscard]] constexpr color() noexcept : v(0.0, 0.0, 0.0, 1.0) {}
    [[nodiscard]] constexpr color(float r, float g, float b, float a = 1.0) noexcept : v(r, g, b, a) {}

    [[nodiscard]] float &r() noexcept { return v.x(); }
    [[nodiscard]] float &g() noexcept { return v.y(); }
    [[nodiscard]] float &b() noexcept { return v.z(); }
    [[nodiscard]] float &a() noexcept { return v.w(); }
    [[nodiscard]] float const &r() const noexcept { return v.x(); }
    [[nodiscard]] float const &g() const noexcept { return v.y(); }
    [[nodiscard]] float const &b() const noexcept { return v.z(); }
    [[nodiscard]] float const &a() const noexcept { return v.w(); }


private:
    f32x4 v;
};


}

