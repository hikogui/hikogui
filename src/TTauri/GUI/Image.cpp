// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/Image.hpp"
#include "TTauri/Foundation/png.hpp"
#include "TTauri/GUI/DrawContext.hpp"
#include "TTauri/GUI/Window.hpp"

namespace tt {

Image::Image(PixelMap<R16G16B16A16SFloat> &&image) noexcept :
    image(std::move(image)) {}

Image::Image(FontGlyphIDs const &image) noexcept :
    image(image) {}

Image::Image(URL const &url) :
    Image(png::load(url)) {}

Image::Image(Image const &other) noexcept
{
    if (auto font_glyph_id = std::get_if<FontGlyphIDs>(&other.image)) {
        image = *font_glyph_id;

    } else if (auto pixmap = std::get_if<PixelMap<R16G16B16A16SFloat>>(&other.image)) {
        image = pixmap->copy();

    } else {
        tt_no_default;
    }
}

Image &Image::operator=(Image const &other) noexcept
{
    if (auto font_glyph_id = std::get_if<FontGlyphIDs>(&other.image)) {
        image = *font_glyph_id;

    } else if (auto pixmap = std::get_if<PixelMap<R16G16B16A16SFloat>>(&other.image)) {
        image = pixmap->copy();

    } else {
        tt_no_default;
    }
    return *this;
}

void Image::prepareForDrawing(Window &window) noexcept
{
    if (auto font_glyph_id = std::get_if<FontGlyphIDs>(&image)) {
        boundingBox = PipelineSDF::DeviceShared::getBoundingBox(*font_glyph_id);

    } else if (auto pixmap = std::get_if<PixelMap<R16G16B16A16SFloat>>(&image)) {
        boundingBox = aarect{pixmap->extent()};
        if (window.device) {
            backing = window.device->imagePipeline->makeImage(pixmap->extent());
            backing.upload(*pixmap);
        }

    } else {
        tt_no_default;
    }
}

bool Image::draw(DrawContext const &drawContext, aarect rectangle) noexcept
{
    auto context = drawContext;
    context.transform =
        context.transform *
        mat::uniform2D_scale_and_translate(rectangle, boundingBox, Alignment::MiddleCenter);

    if (auto font_glyph_id = std::get_if<FontGlyphIDs>(&image)) {
        context.drawGlyph(*font_glyph_id, boundingBox);
        return false;

    } else if (std::holds_alternative<PixelMap<R16G16B16A16SFloat>>(image)) {
        switch (backing.state) {
        case PipelineImage::Image::State::Drawing:
            return true;

        case PipelineImage::Image::State::Uploaded:
            context.drawImage(backing);
            return false;

        default:
            return false;
        }

    } else {
        tt_no_default;
    }
}

}
