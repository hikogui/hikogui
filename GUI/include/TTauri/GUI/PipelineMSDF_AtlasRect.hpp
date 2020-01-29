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
    uint16_t x;
    uint16_t y;
    uint8_t z;
    uint8_t width;
    uint8_t height;

    constexpr AtlasRect(glm::ivec3 position, iextent2 extent) noexcept :
        x(position.x), y(position.y), z(position.z), width(extent.width()), height(extent.height()) {}
};

}
