// Copyright 2020 Pokitec
// All rights reserved.

#include "Image.hpp"
#include "PixelMapCell.hpp"
#include "GlyphCell.hpp"
#include "../encoding/png.hpp"
#include "../GUI/DrawContext.hpp"
#include "../GUI/Window.hpp"

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

[[nodiscard]] std::unique_ptr<ImageCell> Image::makeCell() const noexcept
{
    if (ttlet pixel_map = std::get_if<PixelMap<R16G16B16A16SFloat>>(&image)) {
        return std::make_unique<PixelMapCell>(*pixel_map);
    } else if (ttlet glyph = std::get_if<FontGlyphIDs>(&image)) {
        return std::make_unique<GlyphCell>(*glyph);
    } else {
        tt_no_default;
    }
}


}
