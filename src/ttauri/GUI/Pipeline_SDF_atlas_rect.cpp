// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_SDF_atlas_rect.hpp"
#include "pipeline_SDF_device_shared.hpp"
#include "../mat.hpp"

namespace tt::pipeline_SDF {

atlas_rect::atlas_rect(i32x4 atlasPosition, f32x4 drawExtent) noexcept :
    atlasPosition(atlasPosition),
    atlasExtent(ceil(drawExtent))
{
    ttlet atlas_px_rect = rect{
        f32x4{atlasPosition.xyz1()},
        drawExtent
    };

    ttlet textureCoordinateScale = mat::S{
        device_shared::atlasTextureCoordinateMultiplier,
        device_shared::atlasTextureCoordinateMultiplier
    };

    ttlet atlas_tx_rect = textureCoordinateScale * atlas_px_rect;

    std::get<0>(textureCoords) = atlas_tx_rect.corner<0>();
    std::get<1>(textureCoords) = atlas_tx_rect.corner<1>();
    std::get<2>(textureCoords) = atlas_tx_rect.corner<2>();
    std::get<3>(textureCoords) = atlas_tx_rect.corner<3>();
}

}
