// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ImageCell.hpp"
#include "../text/FontGlyphIDs.hpp"
#include "../aarect.hpp"

namespace tt {

class GlyphCell : public ImageCell {
    FontGlyphIDs glyph;
    mutable aarect boundingBox;

public:
    GlyphCell(FontGlyphIDs glyph);

    GlyphCell(GlyphCell const &) noexcept = delete;
    GlyphCell(GlyphCell &&) noexcept = delete;
    GlyphCell &operator=(GlyphCell const &) noexcept = delete;
    GlyphCell &operator=(GlyphCell &&) noexcept = delete;

    void draw(DrawContext const &drawContext, aarect rectangle, Alignment alignment, float middle) const noexcept override;
};

}
