// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file color/Rec2020.hpp color space conversion matrices between XYZ and Rec.2020
 * @ingroup color
 */

module;
#include "../macros.hpp"

#include <cmath>
#include <array>

export module hikogui_color_Rec2020;
import hikogui_color_color_space;
import hikogui_geometry;

export namespace hi {
inline namespace v1 {

/** Rec.2020 to XYZ color space conversion matrix.
 * @ingroup color
 */
constexpr matrix3 Rec2020_to_XYZ = color_primaries_to_RGBtoXYZ(0.3127f, 0.3290f, 0.708f, 0.292f, 0.170f, 0.797f, 0.131f, 0.046f);

/** XYZ to Rec.2020 color space conversion matrix.
 * @ingroup color
 */
constexpr matrix3 XYZ_to_Rec2020 = ~Rec2020_to_XYZ;

}} // namespace hi::inline v1

