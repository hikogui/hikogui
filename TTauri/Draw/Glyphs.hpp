// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Glyph.hpp"

namespace TTauri::Draw {

struct Glyphs {
    std::vector<Glyph> glyphs;

    /*! Total width of the text.
     * can be called before positionGlyphs().
     */
    float advanceWidth() const {
        float width = 0.0;
        for (let glyph: glyphs) {
            width += glyph.advanceWidth;
        }
        return width;
    }

    /*! Get the cursor position at grapheme index.
     */
    float cursorPosition(size_t graphemeIndex) const {
        float width = 0.0;

        for (let glyph: glyphs) {
            if (graphemeIndex < glyph.numberOfGraphemes) {
                return width + glyph.getAdvanceForGrapheme(graphemeIndex);
            } else {
                width += glyph.advanceWidth;
            }
            graphemeIndex -= glyph.numberOfGraphemes;
        }
        return width;
    }

};

}