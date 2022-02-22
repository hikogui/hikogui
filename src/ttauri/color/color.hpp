// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../rapid/numeric_array.hpp"

namespace tt::inline v1 {

/** This is a RGBA floating point color.
 * The color can be converted between different color spaces using the matrix-class.
 *
 * But in most cases in the application and ttauri library this color would be in
 * the tsRGBA color space. This color space is compatible with the sRGB standard
 * IEC 61966-2-1:1999.
 *
 * tsRGB details:
 * - the ITU-R BT.709 color primaries.
 * - A linear transfer function (unlike sRGB).
 * - R=0.0, G=0.0, B=0.0: Black
 * - R=1.0, G=1.0, B=1.0: White D65 at 80 nits (80 cd/m^2).
 * - RGB values above 1.0 are allowed for HDR (high dynamic range)
 * - RGB values below 0.0 are allowed for WCG (wide color gamut)
 *
 * tsRGBA details:
 * - Includes an alpha value
 * - Alpha values are linear and must be between 0.0 through 1.0.
 * - A=0.0 fully transparent
 * - A=1.0 fully opaque
 * - RGB values are NOT pre-multiplied with the alpha.
 *
 * This color format is inspired by scRGB, however scRGB only describes
 * a 12- or 16-bit integer per component encoding of RGB values between
 * -0.5 and 7.5.
 */
class color {
public:
    constexpr color(color const &) noexcept = default;
    constexpr color(color &&) noexcept = default;
    constexpr color &operator=(color const &) noexcept = default;
    constexpr color &operator=(color &&) noexcept = default;

    [[nodiscard]] constexpr explicit color(f16x4 const &other) noexcept : _v(other)
    {
        tt_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit color(f32x4 const &other) noexcept : color(static_cast<f16x4>(other)) {}

    [[nodiscard]] constexpr explicit operator f16x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return static_cast<f32x4>(_v);
    }

    [[nodiscard]] constexpr color(float r, float g, float b, float a = 1.0f) noexcept : color(f32x4{r, g, b, a}) {}

    [[nodiscard]] constexpr color() noexcept : color(f32x4{}) {}

    [[nodiscard]] static constexpr color transparent() noexcept
    {
        return {0.0f, 0.0f, 0.0f, 0.0f};
    }

    [[nodiscard]] static constexpr color white() noexcept
    {
        return {1.0f, 1.0f, 1.0f, 1.0f};
    }

    [[nodiscard]] static constexpr color black() noexcept
    {
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }

    [[nodiscard]] constexpr float16 &r() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float16 &g() noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float16 &b() noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float16 &a() noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr float16 const &r() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float16 const &g() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float16 const &b() const noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float16 const &a() const noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() >= 0.0 && _v.w() <= 1.0;
    }

    [[nodiscard]] constexpr friend bool operator==(color const &lhs, color const &rhs) noexcept = default;

    [[nodiscard]] constexpr friend color operator*(color const &lhs, color const &rhs) noexcept
    {
        return color{lhs._v * rhs._v};
    }

    [[nodiscard]] constexpr friend color composit(color const &lhs, color const &rhs) noexcept
    {
        return color{composit(lhs._v, rhs._v)};
    }

    [[nodiscard]] constexpr friend color desaturate(color const &rhs) noexcept
    {
        auto rhs_ = f32x4{rhs};

        auto Y = 0.2126f * rhs_.r() + 0.7152f * rhs_.g() + 0.0722f * rhs_.b();

        return color{Y, Y, Y, rhs_.a()};
    }

private:
    f16x4 _v;
};

} // namespace tt::inline v1
