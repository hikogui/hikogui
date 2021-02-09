// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_glyph_ids.hpp"
#include "attributed_grapheme.hpp"
#include "glyph_metrics.hpp"
#include "../mat.hpp"

namespace tt {

/**
*/
struct attributed_glyph {
    font_glyph_ids glyphs;

    /** The logical index of the grapheme before bidi-algorithm.
    */
    ssize_t logicalIndex;

    /** Metrics taken from the font file, pre-scaled to the font-size. */
    glyph_metrics metrics;

    /** Position of the glyph.
    */
    f32x4 position;

    /** Number of graphemes merged (ligature) into this attributed-glyph. */
    int8_t graphemeCount;

    unicode_general_category general_category;

    /** Copied from the original attributed-grapheme. */
    text_style style;

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
    attributed_glyph(attributed_grapheme const &attr_grapheme, attributed_glyph const *next_attr_glyph=nullptr) noexcept;

    attributed_glyph(attributed_glyph const &other) = default;
    attributed_glyph(attributed_glyph &&other) noexcept = default;
    attributed_glyph &operator=(attributed_glyph const &other) = default;
    attributed_glyph &operator=(attributed_glyph &&other) noexcept = default;
    ~attributed_glyph() = default;

    /** Check if this glyph contains the grapheme at index.
     */
    [[nodiscard]] bool containsLogicalIndex(ssize_t index) const noexcept {
        ttlet first = logicalIndex;
        ttlet last = first + graphemeCount;
        return index >= first && index < last;
    }

    [[nodiscard]] bool isLetter() const noexcept
    {
        return is_L(general_category);
    }

    [[nodiscard]] bool isDigit() const noexcept
    {
        return is_N(general_category);
    }

    [[nodiscard]] bool isIdentifier() const noexcept { return isLetter() || isDigit(); }

    [[nodiscard]] bool isWhiteSpace() const noexcept
    {
        return general_category == unicode_general_category::Zs || general_category == unicode_general_category::Zl;
    }

    [[nodiscard]] bool isParagraphSeparator() const noexcept
    {
        return general_category == unicode_general_category::Zp;
    }
    [[nodiscard]] bool isVisible() const noexcept
    {
        return is_visible(general_category);
    }
    
    /** return a cluster id for word selection.
     * This makes clusters of:
     *  - paragraph separator.
     *  - identifiers (letter and digits),
     *  - visibiles (other marks and symbols)
     *  - whitespace
     */
    [[nodiscard]] int selectionWordClusterID() const noexcept {
        if (isParagraphSeparator()) {
            return 0;
        } else if (isIdentifier()) {
            return 1;
        } else if (isVisible()) {
            return 2;
        } else {
            return 3;
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
    [[nodiscard]] ssize_t relativeIndexAtCoordinate(f32x4 coordinate) const noexcept {
        ttlet relativePositionInGlyph = (coordinate.x() - position.x()) / metrics.advance.x();
        ttlet relativePositionPergrapheme = relativePositionInGlyph * narrow_cast<float>(graphemeCount);
        return narrow_cast<ssize_t>(std::round(relativePositionPergrapheme));
    }

    [[nodiscard]] Path get_path() const noexcept;
};

}
