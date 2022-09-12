// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "color_space.hpp"
#include <cmath>
#include <array>

namespace hi::inline v1 {

constexpr matrix3 Rec2020_to_XYZ = color_primaries_to_RGBtoXYZ(0.3127f, 0.3290f, 0.708f, 0.292f, 0.170f, 0.797f, 0.131f, 0.046f);

constexpr matrix3 XYZ_to_Rec2020 = ~Rec2020_to_XYZ;

} // namespace hi::inline v1
