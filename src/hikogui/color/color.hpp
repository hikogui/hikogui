// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "semantic_color.hpp"
#include "../rapid/numeric_array.hpp"
#include "../assert.hpp"

namespace hi::inline v1 {

/** This is a RGBA floating point color.
 * The color can be converted between different color spaces using the matrix-class.
 *
 * But in most cases in the application and hikogui library this color would be in
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
    constexpr color(color const&) noexcept = default;
    constexpr color(color&&) noexcept = default;
    constexpr color& operator=(color const&) noexcept = default;
    constexpr color& operator=(color&&) noexcept = default;

    [[nodiscard]] constexpr explicit color(f16x4 const& other) noexcept : _v(other)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit color(f32x4 const& other) noexcept : color(static_cast<f16x4>(other)) {}

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

    [[nodiscard]] constexpr explicit color(hi::semantic_color semantic_color, float alpha = 1.0f) noexcept : _v()
    {
        _v.x() = float16::from_uint16_t(0xf900 + static_cast<uint8_t>(semantic_color));
        _v.y() = float16::from_uint16_t(0x0000);
        _v.z() = float16::from_uint16_t(0x0000);
        _v.w() = float16(1.0f);
    }

    [[nodiscard]] constexpr bool is_semantic() const noexcept
    {
        return (_v.x().get() & 0xf900) == 0xf900;
    }

    constexpr explicit operator semantic_color() const noexcept
    {
        hi_axiom(is_semantic());
        return static_cast<semantic_color>(_v.x().get() & 0xff);
    }

    // clang-format off
    [[nodiscard]] static constexpr color blue() noexcept { return color{semantic_color::blue}; }
    [[nodiscard]] static constexpr color green() noexcept { return color{semantic_color::green}; }
    [[nodiscard]] static constexpr color indigo() noexcept { return color{semantic_color::indigo}; }
    [[nodiscard]] static constexpr color orange() noexcept { return color{semantic_color::orange}; }
    [[nodiscard]] static constexpr color pink() noexcept { return color{semantic_color::pink}; }
    [[nodiscard]] static constexpr color purple() noexcept { return color{semantic_color::purple}; }
    [[nodiscard]] static constexpr color red() noexcept { return color{semantic_color::red}; }
    [[nodiscard]] static constexpr color teal() noexcept { return color{semantic_color::teal}; }
    [[nodiscard]] static constexpr color yellow() noexcept { return color{semantic_color::yellow}; }
    [[nodiscard]] static constexpr color gray() noexcept { return color{semantic_color::gray}; }
    [[nodiscard]] static constexpr color gray2() noexcept { return color{semantic_color::gray2}; }
    [[nodiscard]] static constexpr color gray3() noexcept { return color{semantic_color::gray3}; }
    [[nodiscard]] static constexpr color gray4() noexcept { return color{semantic_color::gray4}; }
    [[nodiscard]] static constexpr color gray5() noexcept { return color{semantic_color::gray5}; }
    [[nodiscard]] static constexpr color gray6() noexcept { return color{semantic_color::gray6}; }
    [[nodiscard]] static constexpr color foreground() noexcept { return color{semantic_color::foreground}; }
    [[nodiscard]] static constexpr color border() noexcept { return color{semantic_color::border}; }
    [[nodiscard]] static constexpr color fill() noexcept { return color{semantic_color::fill}; }
    [[nodiscard]] static constexpr color accent() noexcept { return color{semantic_color::accent}; }
    [[nodiscard]] static constexpr color text_select() noexcept { return color{semantic_color::text_select}; }
    [[nodiscard]] static constexpr color primary_cursor() noexcept { return color{semantic_color::primary_cursor}; }
    [[nodiscard]] static constexpr color secondary_cursor() noexcept { return color{semantic_color::secondary_cursor}; }
    // clang-format on

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

    [[nodiscard]] size_t hash() const noexcept
    {
        return std::hash<uint64_t>{}(std::bit_cast<uint64_t>(_v));
    }

    [[nodiscard]] constexpr float16& r() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float16& g() noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float16& b() noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float16& a() noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr float16 const& r() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float16 const& g() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float16 const& b() const noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float16 const& a() const noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() >= 0.0 && _v.w() <= 1.0;
    }

    [[nodiscard]] constexpr friend bool operator==(color const& lhs, color const& rhs) noexcept = default;

    [[nodiscard]] constexpr friend color operator*(color const& lhs, color const& rhs) noexcept
    {
        return color{lhs._v * rhs._v};
    }

    [[nodiscard]] constexpr friend color composit(color const& lhs, color const& rhs) noexcept
    {
        return color{composit(lhs._v, rhs._v)};
    }

    [[nodiscard]] constexpr friend color desaturate(color const& rhs) noexcept
    {
        hilet rhs_ = f32x4{rhs};

        hilet Y = 0.2126f * rhs_.r() + 0.7152f * rhs_.g() + 0.0722f * rhs_.b();

        return color{Y, Y, Y, rhs_.a()};
    }

private:
    f16x4 _v;
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::color> {
    [[nodiscard]] size_t operator()(hi::color const &rhs) const noexcept
    {
        return rhs.hash();
    }
};
