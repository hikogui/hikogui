// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/Image.hpp"
#include "TTauri/Foundation/png.hpp"

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


}
