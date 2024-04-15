// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "device_type.hpp"
#ifdef pascal
#undef pascal
#endif
#include <hikothird/au.hh>
#include <ratio>
#include <concepts>
#include <compare>

hi_export_module(hikogui.utility.units);

hi_export namespace hi {
inline namespace v1 {

struct PixelLengthDim : au::base_dim::BaseDimension<1712674722> {};
struct RelativeFontLengthDim : au::base_dim::BaseDimension<1712674734> {};
struct DeviceIndependentPixelLengthDim : au::base_dim::BaseDimension<1712760807> {};

struct Pixels : au::UnitImpl<au::Dimension<PixelLengthDim>> {
    static constexpr inline const char label[] = "px";
};
constexpr auto pixel = au::SingularNameFor<Pixels>{};
constexpr auto pixels = au::QuantityMaker<Pixels>{};
constexpr auto pixels_pt = au::QuantityPointMaker<Pixels>{};
using pixels_d = au::Quantity<Pixels, double>;
using pixels_f = au::Quantity<Pixels, float>;
using pixels_i = au::Quantity<Pixels, int>;

struct PixelsPerInch : decltype(Pixels{} / au::Inches{}) {
    static constexpr const char label[] = "ppi";
};
constexpr auto pixel_per_inch = au::SingularNameFor<PixelsPerInch>{};
constexpr auto pixels_per_inch = au::QuantityMaker<PixelsPerInch>{};
constexpr auto pixels_per_inch_pt = au::QuantityPointMaker<PixelsPerInch>{};
using pixels_per_inch_d = au::Quantity<PixelsPerInch, double>;
using pixels_per_inch_f = au::Quantity<PixelsPerInch, float>;
using pixels_per_inch_i = au::Quantity<PixelsPerInch, int>;



/** Device Independent Pixel.
 *
 * Device Independent Pixels are scaled not only based on the
 * PPI (pixels per inch) of the display, but also based on the viewing distance.
 * This will help make text and other graphic elements equally readable on
 * mobile devices, computers and televisions.
 *
 * The scaling factor for Device Independent Pixels is rounded to improve pixel
 * alignment, example scaling factors are: 0.75, 1.0 (base), 1.5, 2.0, 3.0, 4.0.
 *
 * A Device Independent Pixel is about 1/160th of an inch on a mobile phone.
 */
struct DeviceIndependentPixel : au::UnitImpl<au::Dimension<DeviceIndependentPixelLengthDim>> {
    static constexpr inline const char label[] = "dp";
};
constexpr auto device_independent_pixel = au::SingularNameFor<DeviceIndependentPixel>{};
constexpr auto device_independent_pixels = au::QuantityMaker<DeviceIndependentPixel>{};
constexpr auto device_independent_pixels_pt = au::QuantityPointMaker<DeviceIndependentPixel>{};
using device_independent_pixel_d = au::Quantity<DeviceIndependentPixel, double>;
using device_independent_pixel_f = au::Quantity<DeviceIndependentPixel, float>;
using device_independent_pixel_i = au::Quantity<DeviceIndependentPixel, int>;

struct pixel_density {
    device_type type;
    pixels_per_inch_f ppi;


    [[nodiscard]] constexpr pixels_f to_pixels(points_f rhs) const noexcept
    {
        return rhs * ppi;
    }

    [[nodiscard]] constexpr pixels_f to_pixels(device_indepedent_pixles_f rhs) const noexcept
    {
        return pixels(density_scale() * rhs.in(device_independent_pixles));
    }

private:
    /** Return a density-scale to convert device independet pixels to normal pixels.
     */
    [[nodiscard]] constexpr float density_scale() const noexcept
    {
        // The base density is based on the device type which determines
        // the viewing distance.
        auto const base_density = [type]() {
            switch (type) {
            case device_type::watch:
            case device_type::phone:
            case device_type::tablet:
                // A mobile device is a medium-density display.
                return 160;

            case device_type::desktop:
                // A normal desktop is a low-density display.
                return 120;

            case device_type::game_console:
            case device_type::television:
                // A normal television is 1.33 * medium-density display.
                return 213;

            default:
                hi_no_default();
            }
        }();

        auto const index = static_cast<int>(ppi.in(pixels_per_inch)) * 4 / base_density;
        switch (index) {
        case 0: return 0.5f;
        case 1: return 0.5f;
        case 2: return 0.5f;

        case 3: return 0.75f; // 120 dpi

        case 4: return 1.0f; // 160 dpi
        case 5: return 1.0f;

        case 6: return 1.5f; // 240 dpi
        case 7: return 1.5f;

        case 8: return 2.0f; // 320 dpi
        case 9: return 2.0f;
        case 10: return 2.0f;
        case 11: return 2.0f;

        case 12: return 3.0f; // 480 dpi
        case 13: return 3.0f;
        case 14: return 3.0f;
        case 15: return 3.0f;
        default: return 4.0f; // 640 dpi
        }
    }
};



struct EmSquares : au::UnitImpl<au::Dimension<RelativeFontLengthDim>> {
    static constexpr const char label[] = "em";
};
constexpr auto em_square = au::SingularNameFor<EmSquares>{};
constexpr auto em_squares = au::QuantityMaker<EmSquares>{};
constexpr auto em_squares_pt = au::QuantityPointMaker<EmSquares>{};
using em_squares_d = au::Quantity<EmSquares, double>;
using em_squares_f = au::Quantity<EmSquares, float>;
using em_squares_i = au::Quantity<EmSquares, int>;

struct Points : decltype(au::Inches{} / au::mag<72>()) {
    static constexpr const char label[] = "pt";
};
constexpr auto point = au::SingularNameFor<Points>{};
constexpr auto points = au::QuantityMaker<Points>{};
constexpr auto points_pt = au::QuantityPointMaker<Points>{};
using points_d = au::Quantity<Points, double>;
using points_f = au::Quantity<Points, float>;
using points_i = au::Quantity<Points, int>;
using points_s = au::Quantity<Points, short>;

namespace symbols {
constexpr auto px = au::SymbolFor<Pixels>{};
constexpr auto dp = au::SymbolFor<DeviceIndependentPixel>{};
constexpr auto ppi = au::SymbolFor<PixelsPerInch>{};
constexpr auto em = au::SymbolFor<EmSquares>{};
constexpr auto pt = au::SymbolFor<Points>{};
} // namespace symbols

/** Convert a length relative to the font size to the au::Length dimension.
 *
 * @param length A length, most often denoted in "em".
 * @param font_size The current font size by which to scale the length.
 * @return The scaled length in the au::Length dimension.
 */
template<typename LengthT, typename FontSizeD, typename FontSizeT>
[[nodiscard]] constexpr au::Quantity<FontSizeD, std::common_type_t<LengthT, FontSizeT>>
to_length(au::Quantity<EmSquares, LengthT> length, au::Quantity<FontSizeD, FontSizeT> font_size) noexcept
{
    return length.in(em_squares)*font_size;
}

/** Convert a length into pixels on the current display.
 *
 * @param length A length to be converted to pixels.
 * @param pixel_density The pixel density of the current display often denoted
 *                      in pixels-per-inch (PPI).
 * @return The length; scaled to pixels.
 */
template<typename LengthT, typename LengthD, typename PixelDensityT>
[[nodiscard]] constexpr au::Quantity<Pixels, std::common_type_t<LengthT, PixelDensityT>>
as_pixels(au::Quantity<LengthD, LengthT> length, au::Quantity<PixelsPerInch, PixelDensityT> pixel_density) noexcept
{
    return length * pixel_density;
}

template<typename LengthT, typename LengthD, typename PixelDensityT>
[[nodiscard]] constexpr std::common_type_t<LengthT, PixelDensityT>
in_pixels(au::Quantity<LengthD, LengthT> length, au::Quantity<PixelsPerInch, PixelDensityT> pixel_density) noexcept
{
    return as_pixels(length, pixel_density).in(pixels);
}

class length_f : public std::variant<points_f, pixels_f, device_independent_pixels_f> {
public:
    [[nodiscard]] constexpr pixels_f as_pixels(pixel_density density) const noexcept
    {

    }
};

} // namespace v1
} // namespace hi::v1
