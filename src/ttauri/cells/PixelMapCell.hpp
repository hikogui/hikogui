// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/cells/ImageCell.hpp"
#include "TTauri/GUI/PipelineImage_Image.hpp"
#include "ttauri/foundation/PixelMap.hpp"
#include "ttauri/foundation/R16G16B16A16SFloat.hpp"
#include "ttauri/foundation/URL.hpp"

namespace tt {

class PixelMapCell : public ImageCell {
    PixelMap<R16G16B16A16SFloat> pixelMap;

    mutable PipelineImage::Image backing;

public:
    PixelMapCell(PixelMap<R16G16B16A16SFloat> &&pixelMap);
    PixelMapCell(PixelMap<R16G16B16A16SFloat> const &pixelMap);
    PixelMapCell(URL const &url);

    void draw(DrawContext const &drawContext, aarect rectangle, Alignment alignment, float middle) const noexcept override;
};

}
