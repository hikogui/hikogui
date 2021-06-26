// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_SDF_atlas_rect.hpp"
#include "pipeline_SDF_device_shared.hpp"
#include "../geometry/scale.hpp"

namespace tt::pipeline_SDF {

atlas_rect::atlas_rect(point3 atlas_position, extent2 size) noexcept : atlas_position(atlas_position), size(ceil(size))
{
    ttlet atlas_px_rect = rectangle{atlas_position, size};

    ttlet texture_coordinate_scale = scale2{device_shared::atlasTextureCoordinateMultiplier};

    ttlet atlas_tx_rect = texture_coordinate_scale * atlas_px_rect;

    std::get<0>(texture_coordinates) = get<0>(atlas_tx_rect);
    std::get<1>(texture_coordinates) = get<1>(atlas_tx_rect);
    std::get<2>(texture_coordinates) = get<2>(atlas_tx_rect);
    std::get<3>(texture_coordinates) = get<3>(atlas_tx_rect);
}

} // namespace tt::pipeline_SDF
