// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <limits>
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/geometry.hpp"

namespace TTauri::GUI::PipelineMSDF {

/* A location inside the atlas where the character is located.
 */
struct AtlasRect {
    glm::ivec3 atlas_position;
    iextent2 atlas_extent;

    rect2 bounding_box;
};

}
