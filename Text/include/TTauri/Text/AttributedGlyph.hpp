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

    GeneralCharacterClass charClass;

    /** Copied from the original attributed-grapheme. */
    TextStyle style;

    /** Metrics taken from the font file, pre-scaled to the font-size. */
    GlyphMetrics metrics;

    /** Position of the glyph.
     */
    vec position;

    AttributedGlyph(AttributedGrapheme const &attr_grapheme, FontGlyphIDs glyphs) noexcept :
        glyphs(std::move(glyphs)),
        logicalIndex(attr_grapheme.logicalIndex),
        graphemeCount(1),
        charClass(attr_grapheme.charClass),
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

    [[nodiscard]] bool isLetter() const noexcept { return charClass == GeneralCharacterClass::Letter; }
    [[nodiscard]] bool isDigit() const noexcept { return charClass == GeneralCharacterClass::Digit; }
    [[nodiscard]] bool isWord() const noexcept { return isLetter() || isDigit(); }
    [[nodiscard]] bool isWhiteSpace() const noexcept { return charClass == GeneralCharacterClass::WhiteSpace; }
    [[nodiscard]] bool isParagraphSeparator() const noexcept { return charClass == GeneralCharacterClass::ParagraphSeparator; }

    /** Get the bounding box for this glyph.
     * Get the scaled and positioned bounding box.
     * @param border The 1EM scaled border around the glyph bounding box.
     */
    [[nodiscard]] rect boundingBox(float border) const noexcept {
        return mat::T(position) * expand(metrics.boundingBox, border * style.size);
    }

    /** Find the logical index closest to the coordinate.
     * For a non-ligature, left of the halfway-point returnes the current logicalIndex,
     * right of the halfway-point return the next logicalIndex.
     */
    [[nodiscard]] ssize_t relativeIndexAtCoordinate(vec coordinate) const noexcept {
        let relativePositionInGlyph = (coordinate.x() - position.x()) / metrics.advance.x();
        let relativePositionPerGrapheme = relativePositionInGlyph * numeric_cast<float>(graphemeCount);
        return numeric_cast<ssize_t>(std::round(relativePositionPerGrapheme));
    }

    [[nodiscard]] Path get_path() const noexcept;
};

}