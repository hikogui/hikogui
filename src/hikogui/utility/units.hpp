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

using font_size = mp_units::quantity<mp_units::isq::length[mp_units::international::point], int>;
using length_in_point = mp_units::quantity<mp_units::isq::length[mp_units::international::point]>;

/** A physical dot/pixel on a display/media.
 */
inline constexpr struct pixel : mp_units::named_unit<"pixel", mp_units::kind_of<mp_units::isq::length>> {
} pixel;

/** Pixels per Inch*/
inline constexpr struct ppi : mp_units::named_unit<"ppi", pixel / mp_units::international::inch> {
} ppi;

/** Em-quad: A font's line-height.
 */
inline constexpr struct em_square : mp_units::named_unit<"em", mp_units::international::point / mp_units::international::point> {
} em_square;

} // namespace v1
} // namespace hi::v1
