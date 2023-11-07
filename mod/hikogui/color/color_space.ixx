// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

/** @file color/color_space.hpp Functions to create color conversion matrices.
 * @ingroup color
 */

#include <compare>

export module hikogui_color_color_space;
import hikogui_geometry;

export namespace hi {
inline namespace v1 {

/** Create a color space conversion matrix.
 *
 * Coordinates for color primaries and white-point are in the CIE xy chromaticity
 * coordinate system.
 *
 * @ingroup color
 * @param wx x-coord for the white point.
 * @param wy y-coord for the white point.
 * @param rx x-coord for the red primary.
 * @param ry y-coord for the red primary.
 * @param gx x-coord for the green primary.
 * @param gy y-coord for the green primary.
 * @param bx x-coord for the blue primary.
 * @param by y-coord for the blue primary.
 */
[[nodiscard]] constexpr matrix3
color_primaries_to_RGBtoXYZ(float wx, float wy, float rx, float ry, float gx, float gy, float bx, float by) noexcept
{
    hilet w = vector3{wx, wy, 1.0f - wx - wy};
    hilet r = vector3{rx, ry, 1.0f - rx - ry};
    hilet g = vector3{gx, gy, 1.0f - gx - gy};
    hilet b = vector3{bx, by, 1.0f - bx - by};

    // Calculate white point's tristimulus values from coordinates
    hilet W = vector3{1.0f * (w.x() / w.y()), 1.0f, 1.0f * (w.z() / w.y())};

    // C is the chromaticity matrix.
    hilet C = matrix3{r, g, b};

    // solve tristimulus sums.
    hilet S = scale3{~C * W};

    return C * S;
}

}} // namespace hi::inline v1
