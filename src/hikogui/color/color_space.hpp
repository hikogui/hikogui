// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../rapid/numeric_array.hpp"
#include "../geometry/matrix.hpp"
#include "../geometry/scale.hpp"

namespace hi::inline v1 {

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

} // namespace hi::inline v1
