// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/extent.hpp"
#include "../geometry/point.hpp"
#include <array>

namespace tt::pipeline_SDF {

/* A location inside the atlas where the character is located.
 */
struct atlas_rect {
    point3 atlas_position;
    extent2 size;

    std::array<point3, 4> texture_coordinates;

    atlas_rect(point3 atlas_position, extent2 size) noexcept;
};

}
