// Copyright 2020 Pokitec
// All rights reserved.

#include "Image.hpp"
#include "PixelMapCell.hpp"
#include "GlyphCell.hpp"
#include "../encoding/png.hpp"
#include "../GUI/DrawContext.hpp"
#include "../GUI/Window.hpp"

namespace tt {

Image::Image() noexcept : image(std::monostate{}) {}

Image::Image(PixelMap<R16G16B16A16SFloat> &&image) noexcept : image(std::move(image)) {}

Image::Image(FontGlyphIDs const &image) noexcept : image(image) {}

Image::Image(URL const &url) : Image(png::load(url)) {}

Image::Image(ElusiveIcon const &icon) noexcept : Image(to_FontGlyphIDs(icon)) {}

Image::Image(TTauriIcon const &icon) noexcept : Image(to_FontGlyphIDs(icon)) {}

Image::Image(Image const &other) noexcept
{
    if (auto font_glyph_id = std::get_if<FontGlyphIDs>(&other.image)) {
        image = *font_glyph_id;

    } else if (auto pixmap = std::get_if<PixelMap<R16G16B16A16SFloat>>(&other.image)) {
        image = pixmap->copy();

    } else if (std::holds_alternative<std::monostate>(other.image)) {
        image = std::monostate{};

    } else {
        tt_no_default();
    }
}

Image::Image(Image &&other) noexcept
{
    if (auto font_glyph_id = std::get_if<FontGlyphIDs>(&other.image)) {
        image = std::move(*font_glyph_id);

    } else if (auto pixmap = std::get_if<PixelMap<R16G16B16A16SFloat>>(&other.image)) {
        image = std::move(*pixmap);

    } else if (std::holds_alternative<std::monostate>(other.image)) {
        image = std::monostate{};

    } else {
        tt_no_default();
    }
    other.image = std::monostate{};
}

Image &Image::operator=(Image const &other) noexcept
{
    if (auto font_glyph_id = std::get_if<FontGlyphIDs>(&other.image)) {
        image = *font_glyph_id;

    } else if (auto pixmap = std::get_if<PixelMap<R16G16B16A16SFloat>>(&other.image)) {
        image = pixmap->copy();

    } else if (std::holds_alternative<std::monostate>(other.image)) {
        image = std::monostate{};

    } else {
        tt_no_default();
    }
    return *this;
}

Image &Image::operator=(Image &&other) noexcept
{
    if (auto font_glyph_id = std::get_if<FontGlyphIDs>(&other.image)) {
        image = std::move(*font_glyph_id);

    } else if (auto pixmap = std::get_if<PixelMap<R16G16B16A16SFloat>>(&other.image)) {
        image = std::move(*pixmap);

    } else if (std::holds_alternative<std::monostate>(other.image)) {
        image = std::monostate{};
    
    } else {
        tt_no_default();
    }
    other.image = std::monostate{};
    return *this;
}

[[nodiscard]] std::unique_ptr<ImageCell> Image::makeCell() const noexcept
{
    if (ttlet pixel_map = std::get_if<PixelMap<R16G16B16A16SFloat>>(&image)) {
        return std::make_unique<PixelMapCell>(*pixel_map);
    } else if (ttlet glyph = std::get_if<FontGlyphIDs>(&image)) {
        return std::make_unique<GlyphCell>(*glyph);
    } else {
        tt_no_default();
    }
}

} // namespace tt
