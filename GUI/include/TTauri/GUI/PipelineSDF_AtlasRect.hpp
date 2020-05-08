// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/ivec.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/aarect.hpp"
#include <array>

namespace TTauri::GUI::PipelineSDF {

/* A location inside the atlas where the character is located.
 */
struct AtlasRect {
    ivec atlasPosition;
    ivec atlasExtent;

    std::array<vec,4> textureCoords;

    AtlasRect(ivec atlasPosition, vec drawExtent) noexcept;
};

}
