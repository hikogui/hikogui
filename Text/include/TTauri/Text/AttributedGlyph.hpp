// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/FontGlyphIDs.hpp"
#include "TTauri/Text/AttributedGrapheme.hpp"
#include "TTauri/Text/GlyphMetrics.hpp"

namespace TTauri::Text {

/**
*/
struct AttributedGlyph {
    FontGlyphIDs glyphs;

    Grapheme grapheme;

    /** Copied from the original attributed-grapheme.
    * An attributed-glyph always represents one or more (ligature) graphemes, a grapheme is never split.
    */
    int index;

    /** Number of graphemes merged (ligature) into this attributed-glyph. */
    uint8_t grapheme_count;

    /** Copied from the original attributed-grapheme. */
    TextStyle style;

    /** Metrics taken from the font file. */
    GlyphMetrics metrics;

    glm::vec2 position;

    AttributedGlyph(AttributedGrapheme const &attr_grapheme, FontGlyphIDs glyphs) noexcept :
        glyphs(std::move(glyphs)), grapheme(attr_grapheme.grapheme), index(attr_grapheme.index), style(attr_grapheme.style), metrics() {}
};

}