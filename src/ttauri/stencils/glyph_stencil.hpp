// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "image_stencil.hpp"
#include "../text/FontGlyphIDs.hpp"
#include "../aarect.hpp"
#include "../mat.hpp"

namespace tt {

class glyph_stencil : public image_stencil {
public:
    using super = image_stencil;

    glyph_stencil(alignment alignment, FontGlyphIDs glyph) noexcept;

    void draw(draw_context context, bool use_context_color=false) noexcept override;

private:
    FontGlyphIDs _glyph;
    aarect _glyph_bounding_box;
    mat _glyph_transform;
};

}
