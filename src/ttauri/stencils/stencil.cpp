// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil.hpp"
#include "glyph_stencil.hpp"
#include "pixel_map_stencil.hpp"
#include "text_stencil.hpp"
#include "label_stencil.hpp"

namespace tt {

[[nodiscard]] std::unique_ptr<image_stencil> stencil::make_unique(Alignment alignment, tt::icon const &icon)
{
    if (ttlet pixel_map = std::get_if<PixelMap<R16G16B16A16SFloat>>(&icon.image)) {
        return std::make_unique<pixel_map_stencil>(alignment, *pixel_map);
    } else if (ttlet glyph = std::get_if<FontGlyphIDs>(&icon.image)) {
        return std::make_unique<glyph_stencil>(alignment, *glyph);
    } else {
        tt_no_default();
    }
}

[[nodiscard]] std::unique_ptr<text_stencil> stencil::make_unique(Alignment alignment, std::u8string const &text, TextStyle const &style)
{
    return std::make_unique<text_stencil>(alignment, text, style);
}

[[nodiscard]] std::unique_ptr<label_stencil> stencil::make_unique(Alignment alignment, tt::l10n_label const &label, TextStyle const &style)
{
    return std::make_unique<label_stencil>(alignment, label, style);
}

}
