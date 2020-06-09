// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/FontGlyphIDs.hpp"
#include "TTauri/Text/AttributedGrapheme.hpp"
#include "TTauri/Text/GlyphMetrics.hpp"
#include "TTauri/Foundation/mat.hpp"

namespace TTauri {

/**
*/
struct AttributedGlyph {
    FontGlyphIDs glyphs;

    /** The logical index of the grapheme before bidi-algorithm.
    */
    ssize_t logicalIndex;

    /** Metrics taken from the font file, pre-scaled to the font-size. */
    GlyphMetrics metrics;

    /** Position of the glyph.
    */
    vec position;

    /** Number of graphemes merged (ligature) into this attributed-glyph. */
    int8_t graphemeCount;

    GeneralCharacterClass charClass;

    /** Copied from the original attributed-grapheme. */
    TextStyle style;

    /** Constructed an attributed glyph from an attributed grapheme.
     * When converting a string of graphemes into a string of glyphs you should
     * process the graphemes in reverse order so that you can pass the next_attr_glyph
     * to this constructor for font-based-kerning.
     *
     * The string of graphemes should already be in display-order; after Unicode-bidi-algorithm.
     *
     * @param attr_grapheme The grapheme to turn into a glyph.
     * @param next_attr_glyph The next glyph in display-ordering, used for kerning.
     */
    AttributedGlyph(AttributedGrapheme const &attr_grapheme, AttributedGlyph const *next_attr_glyph=nullptr) noexcept;

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
    [[nodiscard]] bool isVisible() const noexcept { return isWord() || charClass == GeneralCharacterClass::Unknown; }
    
    /** return a cluster id for word selection.
     * This makes clusters of:
     *  - words (letter and digits),
     *  - visibiles (other visible letters)
     *  - whitespace
     *  - paragraph separator.
     */
    [[nodiscard]] int selectionWordClusterID() const noexcept {
        switch (charClass) {
        case GeneralCharacterClass::ParagraphSeparator: return 0;
        case GeneralCharacterClass::Digit:
        case GeneralCharacterClass::Letter: return 1;
        case GeneralCharacterClass::Unknown: return 2;
        case GeneralCharacterClass::WhiteSpace: return 3;
        default: no_default;
        }
    }

    /** Get the bounding box for this glyph.
     * Get the scaled and positioned bounding box.
     * @param border The 1EM scaled border around the glyph bounding box.
     */
    [[nodiscard]] aarect boundingBox(float border) const noexcept {
        return mat::T2(position.xyz0()) * expand(metrics.boundingBox, border * style.scaled_size());
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
