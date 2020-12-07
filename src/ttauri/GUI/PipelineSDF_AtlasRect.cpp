// Copyright 2020 Pokitec
// All rights reserved.

#include "PipelineSDF_AtlasRect.hpp"
#include "PipelineSDF_DeviceShared.hpp"
#include "../mat.hpp"

namespace tt::PipelineSDF {

AtlasRect::AtlasRect(i32x4 atlasPosition, f32x4 drawExtent) noexcept :
    atlasPosition(atlasPosition),
    atlasExtent(ceil(drawExtent))
{
    ttlet atlas_px_rect = rect{
        f32x4{atlasPosition.xyz1()},
        drawExtent
    };

    ttlet textureCoordinateScale = mat::S{
        DeviceShared::atlasTextureCoordinateMultiplier,
        DeviceShared::atlasTextureCoordinateMultiplier
    };

    ttlet atlas_tx_rect = textureCoordinateScale * atlas_px_rect;

    std::get<0>(textureCoords) = atlas_tx_rect.corner<0>();
    std::get<1>(textureCoords) = atlas_tx_rect.corner<1>();
    std::get<2>(textureCoords) = atlas_tx_rect.corner<2>();
    std::get<3>(textureCoords) = atlas_tx_rect.corner<3>();
}

}
