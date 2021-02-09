// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mat.hpp"
#include <cmath>
#include <array>

namespace tt {

inline mat Rec2020_to_XYZ = mat::RGBtoXYZ(
    0.3127f, 0.3290f,
    0.708f, 0.292f,
    0.170f, 0.797f,
    0.131f, 0.046f
);

inline mat XYZ_to_Rec2020 = ~Rec2020_to_XYZ;



}
