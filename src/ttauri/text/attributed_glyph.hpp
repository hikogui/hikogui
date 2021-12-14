// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_ids.hpp"
#include "attributed_grapheme.hpp"
#include "glyph_metrics.hpp"
#include "font_metrics.hpp"
#include "font.hpp"
#include "../graphic_path.hpp"
#include "../geometry/translate.hpp"

namespace tt::inline v1 {

/**
 */
struct attributed_glyph {
    glyph_ids glyphs;

    /** The logical index of the grapheme before bidi-algorithm.
     */
    ssize_t logicalIndex;

    /** The scale value used to convert 'em' units to the units used in this object.
     */
    float scale;

    /** Metrics taken from the font file.
     *
     * @note scaled.
     */
    glyph_metrics metrics;

    /** Position of the glyph.
     */
    point2 position;

    unicode_bidi_class bidi_class;

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
     * @param font_book The font book to use to look up the glyphs from the given grapheme.
     * @param attr_grapheme The grapheme to turn into a glyph.
     * @param next_attr_glyph The next glyph in display-ordering, used for kerning.
     */
    attributed_glyph(
        tt::font_book const &font_book,
        attributed_grapheme const &attr_grapheme,
        attributed_glyph const *next_attr_glyph = nullptr) noexcept;

    attributed_glyph(attributed_glyph const &other) = default;
    attributed_glyph(attributed_glyph &&other) noexcept = default;
    attributed_glyph &operator=(attributed_glyph const &other) = default;
    attributed_glyph &operator=(attributed_glyph &&other) noexcept = default;
    ~attributed_glyph() = default;

    /** Check if this glyph contains the grapheme at index.
     */
    [[nodiscard]] bool containsLogicalIndex(ssize_t index) const noexcept
    {
        ttlet first = logicalIndex;
        ttlet last = first + narrow_cast<ssize_t>(glyphs.num_graphemes());
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

    [[nodiscard]] bool isIdentifier() const noexcept
    {
        return isLetter() || isDigit();
    }

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
     *  - visibles (other marks and symbols)
     *  - whitespace
     */
    [[nodiscard]] int selectionWordClusterID() const noexcept
    {
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
    [[nodiscard]] aarectangle boundingBox() const noexcept
    {
        return translate2{position} * metrics.bounding_rectangle;
    }

    /** Find the logical index closest to the coordinate.
     * For a non-ligature, left of the halfway-point returns the current logicalIndex,
     * right of the halfway-point return the next logicalIndex.
     */
    [[nodiscard]] ssize_t relativeIndexAtCoordinate(point2 coordinate) const noexcept
    {
        ttlet relativePositionInGlyph = (coordinate.x() - position.x()) / metrics.advance.x();
        ttlet relativePositionPergrapheme = relativePositionInGlyph * narrow_cast<float>(glyphs.num_graphemes());
        return narrow_cast<ssize_t>(std::round(relativePositionPergrapheme));
    }

    /** Advance to the start of the grapheme within the glyph.
     *
     * @param index The index of the grapheme to advance to. The index may be one beyond the last grapheme.
     * @return The advance to get to the grapheme within the glyph.
     */
    [[nodiscard]] vector2 advance_for_grapheme(size_t index) const noexcept
    {
        tt_axiom(index <= glyphs.num_graphemes());
        return (static_cast<float>(index) / static_cast<float>(glyphs.num_graphemes())) * metrics.advance;
    }

    /** Get the font metrics for this attributes glyph.
     *
     * @note: scaled.
     * @return The scaled font_metrics.
     */
    [[nodiscard]] constexpr tt::font_metrics font_metrics() const noexcept
    {
        return scale * glyphs.font().metrics;
    }

    [[nodiscard]] graphic_path get_path() const noexcept;
};

} // namespace tt::inline v1
