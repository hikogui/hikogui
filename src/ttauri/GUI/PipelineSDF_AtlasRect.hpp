// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/required.hpp"
#include "ttauri/ivec.hpp"
#include "ttauri/vec.hpp"
#include "ttauri/aarect.hpp"
#include <array>

namespace tt::PipelineSDF {

/* A location inside the atlas where the character is located.
 */
struct AtlasRect {
    ivec atlasPosition;
    ivec atlasExtent;

    std::array<vec,4> textureCoords;

    AtlasRect(ivec atlasPosition, vec drawExtent) noexcept;
};

}
