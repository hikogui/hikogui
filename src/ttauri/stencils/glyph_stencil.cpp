// Copyright 2020 Pokitec
// All rights reserved.

#include "glyph_stencil.hpp"
#include "../GUI/DrawContext.hpp"
#include "../GUI/PipelineSDF_DeviceShared.hpp"

namespace tt {

glyph_stencil::glyph_stencil(alignment alignment, FontGlyphIDs glyph) noexcept : super(alignment), _glyph(std::move(glyph)) {}

void glyph_stencil::draw(DrawContext context, bool use_context_color) noexcept
{
    if (std::exchange(_data_is_modified, false)) {
        _glyph_bounding_box = PipelineSDF::DeviceShared::getBoundingBox(_glyph);
        _size_is_modified = true;
        _position_is_modified = true;
    }

    auto layout_is_modified = std::exchange(_size_is_modified, false);
    layout_is_modified |= std::exchange(_position_is_modified, false);
    if (layout_is_modified) {
        _glyph_transform = mat::uniform2D_scale_and_translate(_rectangle, _glyph_bounding_box, _alignment);
    }

    context.transform = context.transform * _glyph_transform;
    context.drawGlyph(_glyph, _glyph_bounding_box);
}

} // namespace tt
