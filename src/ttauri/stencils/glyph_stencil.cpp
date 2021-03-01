// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "glyph_stencil.hpp"
#include "../GUI/draw_context.hpp"
#include "../GUI/pipeline_SDF_device_shared.hpp"

namespace tt {

glyph_stencil::glyph_stencil(alignment alignment, font_glyph_ids glyph) noexcept : super(alignment), _glyph(std::move(glyph)) {}

void glyph_stencil::draw(draw_context context, tt::color color, matrix3 transform) noexcept
{
    if (std::exchange(_data_is_modified, false)) {
        _glyph_bounding_box = pipeline_SDF::device_shared::getBoundingBox(_glyph);
        _size_is_modified = true;
        _position_is_modified = true;
    }

    auto layout_is_modified = std::exchange(_size_is_modified, false);
    layout_is_modified |= std::exchange(_position_is_modified, false);
    if (layout_is_modified) {
        _glyph_transform = matrix2::uniform(_glyph_bounding_box, _rectangle, _alignment);
    }

    context.draw_glyph(_glyph, transform * _glyph_transform * _glyph_bounding_box, color);
}

} // namespace tt
