// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../numeric_array.hpp"
#include "../aarect.hpp"
#include <array>

namespace tt::pipeline_SDF {

/* A location inside the atlas where the character is located.
 */
struct atlas_rect {
    i32x4 atlasPosition;
    i32x4 atlasExtent;

    std::array<f32x4,4> textureCoords;

    atlas_rect(i32x4 atlasPosition, f32x4 drawExtent) noexcept;
};

}
