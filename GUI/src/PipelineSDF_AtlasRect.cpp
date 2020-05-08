// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineSDF_AtlasRect.hpp"
#include "TTauri/GUI/PipelineSDF_DeviceShared.hpp"

namespace TTauri::GUI::PipelineSDF {

AtlasRect::AtlasRect(ivec atlasPosition, vec drawExtent) noexcept :
    atlasPosition(atlasPosition),
    atlasExtent(ceil(drawExtent))
{
    let atlas_px_offset = static_cast<vec>(atlasPosition.xy00());
    let atlas_px_extent = drawExtent;
    let atlas_z = numeric_cast<float>(atlasPosition.z());

    let atlas_tx_box = rect{
        atlas_px_offset * DeviceShared::atlasTextureCoordinateMultiplier,
        atlas_px_extent * DeviceShared::atlasTextureCoordinateMultiplier
    };
    get<0>(textureCoords) = atlas_tx_box.corner<0>(atlas_z);
    get<1>(textureCoords) = atlas_tx_box.corner<1>(atlas_z);
    get<2>(textureCoords) = atlas_tx_box.corner<2>(atlas_z);
    get<3>(textureCoords) = atlas_tx_box.corner<3>(atlas_z);
}

}