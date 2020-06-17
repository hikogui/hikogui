// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Cells/ImageCell.hpp"
#include "TTauri/GUI/PipelineImage_Image.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include "TTauri/Foundation/URL.hpp"

namespace tt {

class PixelMapCell : public ImageCell {
    PixelMap<R16G16B16A16SFloat> pixelMap;

    /** True when the pixel map has updated since last prepare.
     */
    bool updated;

    PipelineImage::Image backing;

public:
    PixelMapCell(PixelMap<R16G16B16A16SFloat> &&pixelMap);
    PixelMapCell(PixelMap<R16G16B16A16SFloat> const &pixelMap);
    PixelMapCell(URL const &url);

    PixelMapCell(PixelMapCell const &) noexcept = delete;
    PixelMapCell(PixelMapCell &&) noexcept = delete;
    PixelMapCell &operator=(PixelMapCell const &) noexcept = delete;
    PixelMapCell &operator=(PixelMapCell &&) noexcept = delete;

    void prepareForDrawing(Window &device) noexcept override;

    [[nodiscard]] bool draw(DrawContext const &drawContext, aarect rectangle, Alignment alignment) noexcept override;
};

}