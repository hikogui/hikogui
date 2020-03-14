// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/ivec.hpp"
#include "TTauri/Foundation/vec.hpp"
#include <array>

namespace TTauri::GUI::PipelineSDF {

/* A location inside the atlas where the character is located.
 */
struct AtlasRect {
    ivec atlas_position;
    ivec atlas_extent;

    std::array<vec,4> textureCoords;
};

}
