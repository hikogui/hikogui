// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/FontGlyphIDs.hpp"
#include "TTauri/Text/AttributedGrapheme.hpp"
#include "TTauri/Text/GlyphMetrics.hpp"
#include "TTauri/Foundation/mat.hpp"

namespace TTauri::Text {

/**
*/
struct AttributedGlyph {
    FontGlyphIDs glyphs;

    /** The logical index of the grapheme before bidi-algorithm.
    */
    ssize_t logicalIndex;

    /** Number of graphemes merged (ligature) into this attributed-glyph. */
    int8_t graphemeCount;

    /** This glyph represents a breakable-whitespace
    */
    bool breakableWhitespace;

    /** This the end-of-paragraph marker.
     */
    bool endOfParagraph;

    /** Copied from the original attributed-grapheme. */
    TextStyle style;

    /** Metrics taken from the font file. */
    GlyphMetrics metrics;

    /** Position of the glyph.
     */
    vec position;

    AttributedGlyph(AttributedGrapheme const &attr_grapheme, FontGlyphIDs glyphs) noexcept :
        glyphs(std::move(glyphs)),
        logicalIndex(attr_grapheme.logicalIndex),
        graphemeCount(1),
        breakableWhitespace(attr_grapheme.grapheme == ' '),
        endOfParagraph(attr_grapheme.grapheme == '\n'),
        style(attr_grapheme.style),
        metrics() {}

    AttributedGlyph(AttributedGlyph const &other) = default;
    AttributedGlyph(AttributedGlyph &&other) noexcept = default;
    AttributedGlyph &operator=(AttributedGlyph const &other) = default;
    AttributedGlyph &operator=(AttributedGlyph &&other) noexcept = default;
    ~AttributedGlyph() = default;

    /** Check if this glyph contains the grapheme at index.
     */
    [[nodiscard]] bool containsLogicalIndex(ssize_t index) const noexcept {
        let first = logicalIndex;
        let last = first + graphemeCount;
        return index >= first && index < last;
    }

    [[nodiscard]] Path get_path() const noexcept;
};

}