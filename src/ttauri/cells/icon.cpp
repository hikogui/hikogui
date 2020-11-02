// Copyright 2020 Pokitec
// All rights reserved.

#include "icon.hpp"
#include "PixelMapCell.hpp"
#include "GlyphCell.hpp"
#include "../encoding/png.hpp"
#include "../GUI/DrawContext.hpp"
#include "../GUI/Window.hpp"

namespace tt {

icon::icon() noexcept : image(std::monostate{}) {}

icon::icon(PixelMap<R16G16B16A16SFloat> &&image) noexcept : image(std::move(image)) {}

icon::icon(FontGlyphIDs const &image) noexcept : image(image) {}

icon::icon(URL const &url) : icon(png::load(url)) {}

icon::icon(ElusiveIcon const &icon) noexcept : icon(to_FontGlyphIDs(icon)) {}

icon::icon(TTauriIcon const &icon) noexcept : icon(to_FontGlyphIDs(icon)) {}

icon::icon(icon const &other) noexcept
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

icon &icon::operator=(icon const &other) noexcept
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



[[nodiscard]] std::unique_ptr<ImageCell> icon::makeCell() const noexcept
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
