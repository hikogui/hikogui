// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/required.hpp"
#include "ttauri/foundation/ivec.hpp"
#include "ttauri/foundation/vec.hpp"
#include "ttauri/foundation/aarect.hpp"
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
