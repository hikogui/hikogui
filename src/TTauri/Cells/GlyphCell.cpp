// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Cells/GlyphCell.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/DrawContext.hpp"
#include "TTauri/GUI/PipelineSDF_DeviceShared.hpp"

namespace tt {

GlyphCell::GlyphCell(FontGlyphIDs glyph) :
    glyph(std::move(glyph)) {}

bool GlyphCell::draw(DrawContext const &drawContext, aarect rectangle, Alignment alignment, float middle) const noexcept
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
    return false;
}

}