// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/AttributedGlyph.hpp"
#include "TTauri/Foundation/Path.hpp"

namespace TTauri::Text {

[[nodiscard]] Path AttributedGlyph::get_path() const noexcept
{
    let M = mat::T(position) * mat::S(style.size, style.size);

    auto r = M * glyphs.get_path();
    r.closeLayer(style.color);
    return r;
}

}