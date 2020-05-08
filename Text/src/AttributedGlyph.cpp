// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/AttributedGlyph.hpp"
#include "TTauri/Foundation/Path.hpp"

namespace TTauri::Text {

[[nodiscard]] Path AttributedGlyph::get_path() const noexcept
{
    let M = mat::T(position) * mat::S(style.size, style.size);

    auto [glyph_path, glyph_bounding_box] = glyphs.getPathAndBoundingBox();
    auto transformed_glyph_path = M * glyph_path;
    transformed_glyph_path.closeLayer(style.color);
    return transformed_glyph_path;
}

}