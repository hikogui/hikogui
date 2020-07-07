// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/float16.hpp"
#include "TTauri/Foundation/mat.hpp"
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
