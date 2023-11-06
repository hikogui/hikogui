// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file color/Rec2100.hpp color space conversion matrices between XYZ and Rec.2100
 * @ingroup color
 */

module;
#include "../macros.hpp"

#include <cmath>
#include <array>

export module hikogui_color_Rec2100;
import hikogui_color_Rec2020;
import hikogui_geometry;

export namespace hi { inline namespace v1 {

/** Rec.2100 to XYZ color space conversion matrix.
 * @ingroup color
 */
constexpr matrix3 Rec2100_to_XYZ = Rec2020_to_XYZ;

/** XYZ to Rec.2100 color space conversion matrix.
 * @ingroup color
 */
constexpr matrix3 XYZ_to_Rec2100 = XYZ_to_Rec2020;

/** Rec.2100 linear to gamma transfer function.
 *
 * @ingroup color
 * @param L The linear color value, between 0.0 and 1.0.
 * @return The color value converted to the Rec.2100 gamma corrected value between 0.0 and 1.0.
 */
[[nodiscard]] float Rec2100_linear_to_gamma(float L) noexcept
{
    constexpr float c1 = 0.8359375f;
    constexpr float c2 = 18.8515625f;
    constexpr float c3 = 18.6875f;
    constexpr float m1 = 0.1593017578125f;
    constexpr float m2 = 78.84375;

    hilet Lm1 = std::pow(L, m1);

    return std::pow((c1 + c2 * Lm1) / (1.0f + c3 * Lm1), m2);
}

/** Rec.2100 gamma to linear transfer function.
 *
 * @ingroup color
 * @param N The Rec.2100 gamma corrected color value, between 0.0 and 1.0.
 * @return The color value converted to a linear color value between 0.0 and 1.0.
 */
[[nodiscard]] float Rec2100_gamma_to_linear(float N) noexcept
{
    constexpr float c1 = 0.8359375f;
    constexpr float c2 = 18.8515625f;
    constexpr float c3 = 18.6875f;
    constexpr float m1 = 0.1593017578125f;
    constexpr float m2 = 78.84375;

    hilet Nm2 = std::pow(N, 1.0f / m2);

    return std::pow((Nm2 - c1) / (c2 - c3 * Nm2), 1.0f / m1);
}

}} // namespace hi::v1
