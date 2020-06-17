// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Cells/PixelMapCell.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/DrawContext.hpp"
#include "TTauri/Foundation/png.hpp"

namespace tt {

PixelMapCell::PixelMapCell(PixelMap<R16G16B16A16SFloat> &&pixelMap) :
    pixelMap(std::move(pixelMap)), updated(true) {}

PixelMapCell::PixelMapCell(PixelMap<R16G16B16A16SFloat> const &pixelMap) :
    pixelMap(pixelMap.copy()), updated(true) {}

PixelMapCell::PixelMapCell(URL const &url) :
    PixelMapCell(png::load(url)) {}

void PixelMapCell::prepareForDrawing(Window &window) noexcept
{
    if (window.device && updated) {
        backing = window.device->imagePipeline->makeImage(pixelMap.extent());
        backing.upload(pixelMap);
        updated = false;
    }
}

bool PixelMapCell::draw(DrawContext const &drawContext, aarect rectangle, Alignment alignment) noexcept
{
    ttlet boundingBox = aarect{backing.extent};

    auto context = drawContext;
    context.transform =
        context.transform *
        mat::uniform2D_scale_and_translate(rectangle, boundingBox, alignment);

    switch (backing.state) {
    case PipelineImage::Image::State::Drawing:
        return true;

    case PipelineImage::Image::State::Uploaded:
        context.drawImage(backing);
        return false;

    default:
        return false;
    }
}

}