// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "cast.hpp"
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

} // namespace v1
} // namespace hi::v1
