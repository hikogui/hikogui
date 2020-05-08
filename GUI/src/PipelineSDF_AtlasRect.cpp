// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineSDF_AtlasRect.hpp"
#include "TTauri/GUI/PipelineSDF_DeviceShared.hpp"
#include "TTauri/Foundation/mat.hpp"

namespace TTauri::GUI::PipelineSDF {

AtlasRect::AtlasRect(ivec atlasPosition, vec drawExtent) noexcept :
    atlasPosition(atlasPosition),
    atlasExtent(ceil(drawExtent))
{
    let atlas_px_rect = rect{
        vec{atlasPosition.xyz1()},
        drawExtent
    };

    let textureCoordinateScale = mat::S{
        DeviceShared::atlasTextureCoordinateMultiplier,
        DeviceShared::atlasTextureCoordinateMultiplier
    };

    let atlas_tx_rect = textureCoordinateScale * atlas_px_rect;

    get<0>(textureCoords) = atlas_tx_rect.corner<0>();
    get<1>(textureCoords) = atlas_tx_rect.corner<1>();
    get<2>(textureCoords) = atlas_tx_rect.corner<2>();
    get<3>(textureCoords) = atlas_tx_rect.corner<3>();
}

}