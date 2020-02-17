// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/geometry.hpp"
#include <array>

namespace TTauri::GUI::PipelineSDF {

/* A location inside the atlas where the character is located.
 */
struct AtlasRect {
    glm::ivec3 atlas_position;
    iextent2 atlas_extent;

    std::array<glm::vec3,4> textureCoords;
};

}
