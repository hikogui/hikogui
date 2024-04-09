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

struct Pixels : au::UnitImpl<au::Dimension<PixelLengthDim>> {
    static constexpr inline const char label[] = "px";
};
constexpr auto pixel = au::SingularNameFor<Pixels>{};
constexpr auto pixels = au::QuantityMaker<Pixels>{};

struct PixelsPerInch : decltype(Pixels{} / au::Inches{}) {
    static constexpr const char label[] = "ppi";
};
constexpr auto pixel_per_inch = au::SingularNameFor<PixelsPerInch>{};
constexpr auto pixels_per_inch = au::QuantityMaker<PixelsPerInch>{};
constexpr auto pixels_per_inch_pt = au::QuantityPointMaker<PixelsPerInch>{};

struct EmSquares : au::UnitImpl<au::Dimension<RelativeFontLengthDim>> {
    static constexpr const char label[] = "em";
};
constexpr auto em_square = au::SingularNameFor<EmSquares>{};
constexpr auto em_squares = au::QuantityMaker<EmSquares>{};
constexpr auto em_squares_pt = au::QuantityPointMaker<EmSquares>{};

struct Points : decltype(au::Inches{} / au::mag<72>()) {
    static constexpr const char label[] = "pt";
};
constexpr auto point = au::SingularNameFor<Points>{};
constexpr auto points = au::QuantityMaker<Points>{};
constexpr auto points_pt = au::QuantityPointMaker<Points>{};

namespace symbols {
constexpr auto px = au::SymbolFor<Pixels>{};
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
    return length.in(points)*font_size;
}

/** Convert a length into pixels on the current display.
 *
 * @param length A length to be converted to pixels.
 * @param pixel_density The pixel density of the current display often denoted
 *                      in pixels-per-inch (DPI / PPI).
 * @return The length; scaled to pixels.
 */
template<typename LengthT, typename LengthD, typename PixelDensityT>
[[nodiscard]] constexpr au::Quantity<Pixels, std::common_type_t<LengthT, PixelDensityT>>
as_pixels(au::Quantity<LengthT, LengthD> length, au::Quantity<PixelsPerInch, PixelDensityT> pixel_density) noexcept
{
    return length * pixel_density;
}

template<typename LengthT, typename LengthD, typename PixelDensityT>
[[nodiscard]] constexpr au::Quantity<Pixels, std::common_type_t<LengthT, PixelDensityT>>
in_pixels(au::Quantity<LengthT, LengthD> length, au::Quantity<PixelsPerInch, PixelDensityT> pixel_density) noexcept
{
    return as_pixels(length, pixel_density).in(pixels);
}

} // namespace v1
} // namespace hi::v1
