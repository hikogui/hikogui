// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/theme.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/geometry.hpp"

namespace TTauri {

struct AttributedGrapheme {
    grapheme g;
    int index;

    /** All information about the shape and color needed to render this grapheme. */
    TextStyle style;
};

/**
 */
struct AttributedGlyph {
    FontGlyphIDs glyphs;

    /** Copied from the original attributed-grapheme.
     * An attributed-glyph always represents one or more (ligature) graphemes, a grapheme is never split.
     */
    int index;

    /** Number of graphemes merged (ligature) into this attributed-glyph. */
    uint8_t grapheme_count;

    /** Copied from the original attributed-grapheme. */
    TextStyle style;

    /** Advance vector to the next glyph */
    glm::vec2 advance;

    /** Position and size of the glyph inside the resulting box. */
    rect2 bounding_box;

};

/** Shape the text.
 * The given text is in logical-order; the order in which humans write text.
 * The resulting glyphs are in left-to-right display order.
 *
 * The following operations are executed on the text by the `shape_text()` function:
 *  - Put graphemes in left-to-right display order using the UnicodeData's bidi_algorithm.
 *  - Convert attributed-graphemes into attributes-glyphs using FontBook's find_glyph algorithm.
 *  - Morph attributed-glyphs using the Font's morph algorithm.
 *  - Calculate advance for each attributed-glyph using the Font's advance and kern algorithms.
 *  - Add line-breaks to the text to fit within the maximum-width.
 *  - Calculate actual size of the box, no smaller than the minimum_size.
 *  - Align the text within the actual box size.
 *
 * @param text The text to be shaped.
 * @param max_width Maximum width that the text should flow into.
 * @param alignment How the text should be aligned in the box.
 * @return size of the resulting text, shaped text.
 */
[[nodiscard]] std::pair<extent2,std::vector<AttributedGlyph>> shape_text(std::vector<AttributedGrapheme> text, Alignment alignment, extent2 minimum_size, extent2 maximum_size) noexcept;


}