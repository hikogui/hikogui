// Copyright 2020 Pokitec
// All rights reserved.

#include "GlyphCell.hpp"
#include "../GUI/Window.hpp"
#include "../GUI/DrawContext.hpp"
#include "../GUI/PipelineSDF_DeviceShared.hpp"

namespace tt {

GlyphCell::GlyphCell(FontGlyphIDs glyph) :
    glyph(std::move(glyph)) {}

void GlyphCell::draw(DrawContext const &drawContext, aarect rectangle, Alignment alignment, float middle, bool useContextColor) const noexcept
{
    if (modified) {
        boundingBox = PipelineSDF::DeviceShared::getBoundingBox(glyph);
        modified = false;
    }

    auto context = drawContext;
    context.transform =
        context.transform *
        mat::uniform2D_scale_and_translate(rectangle, boundingBox, alignment);

    context.drawGlyph(glyph, boundingBox);
}

}
