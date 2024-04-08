// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "cast.hpp"
#ifdef pascal
#undef pascal
#endif
#include <mp-units/systems/isq/isq.h>
#include <mp-units/systems/international/international.h>
#include <ratio>
#include <concepts>
#include <compare>

hi_export_module(hikogui.utility.units);

hi_export namespace hi {
inline namespace v1 {

QUANTITY_SPEC(size_in_pixels, mp_units::isq::length, mp_units::is_kind);
QUANTITY_SPEC(font_size, mp_units::isq::length);
QUANTITY_SPEC(relative_font_size, mp_units::isq::length, mp_units::is_kind);
QUANTITY_SPEC(pixel_density, size_in_pixels / mp_units::isq::length);

/** A physical dot/pixel on a display/media.
 */
inline constexpr struct pixel : mp_units::named_unit<"pixel", mp_units::kind_of<size_in_pixels>> {
} pixel;

/** Pixels per Inch
*/
inline constexpr struct pixels_per_inch : mp_units::named_unit<"ppi", pixel / mp_units::international::inch, mp_units::kind_of<pixel_density>> {
} pixels_per_inch;

/** Em-quad: A font's line-height.
 */
inline constexpr struct em_square : mp_units::named_unit<"em", mp_units::kind_of<relative_font_size>> {
} em_square;

/** Convert a length relative to the font size to a SI length.
 * 
 * @param length_relative_to_font_size A length, most often denoted in "em".
 * @param font_size The current font size by which to scale the length.
 * @return The scaled length as a SI length quantity.
*/
template<mp_units::QuantityOf<relative_font_size> RelativeFontSize, mp_units::QuantityOf<font_size> FontSize>
[[nodiscard]] constexpr auto to_length(RelativeFontSize length_relative_to_font_size, FontSize font_size) noexcept
{
    return mp_units::quantity_cast<mp_units::isq::length>(length_relative_to_font_size.numerical_value_in(length_relative_to_font_size.unit) * font_size);
}

/** Convert a SI length into the size in pixels on the current display.
 * 
 * @param length A SI length to be converted to pixels.
 * @param pixel_density The pixel density of the current display often denoted
 *                      in pixels-per-inch (DPI / PPI).
 * @return The length; scaled to pixels.
*/
template<mp_units::QuantityOf<mp_units::isq::length> Length, mp_units::QuantityOf<pixel_density> PixelDensity>
[[nodiscard]] constexpr auto to_pixel(Length length, PixelDensity pixel_density) noexcept
{
    return length * pixel_density;
}

inline constexpr auto px = pixel;
inline constexpr auto ppi = pixels_per_inch;
inline constexpr auto em = em_square;

} // namespace v1
} // namespace hi::v1
