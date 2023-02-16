// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_theme.hpp"
#include "character.hpp"
#include "../font/module.hpp"
#include "../unicode/unicode_description.hpp"
#include "../unicode/grapheme.hpp"
#include "../geometry/module.hpp"

namespace hi::inline v1 {
class font_book;

class text_shaper_char {
public:
    /** The original character.
     */
    hi::character character = {};

    /** The character after replacing bidi mirror glyphs.
     */
    hi::character bidi_character = {};

    /** The resolved style.
     */
    hi::text_style style = {};

    /** The font resolved for this character.
     */
    hi::font const *font = nullptr;

    /** The width of the grapheme before glyph-morphing and positioning.
     */
    float width = 0.0f;

    /** Position of the character.
     */
    point2 position = {};

    /** Advance after glyph-morphing and positioning.
     */
    float advance = 0.0f;

    /** The rectangle for this character.
     *
     * The rectangle is used for:
     *  - creating a selection box around the character.
     *  - creating cursors before, after and on the character.
     *  - converting mouse-position to character.
     *
     * The attributes of the rectangle are:
     *  - left side is equal to the position.x
     *  - The width is the advance of the grapheme within the ligature.
     *  - The bottom is at the descender
     *  - The top is at the ascender
     *
     * When multiple characters are converted to a ligature, the
     * rectangle of each of those characters occupies a
     * subsection of the ligature-glyph. In this case the left most
     * character will contain the ligature-glyph, and the rest of
     * the characters of the ligature will have empty glyphs.
     */
    aarectangle rectangle = {};

    /** The glyph representing this grapheme.
     *
     * It is possible for this grapheme not to have any glyphs when the glyphs where merged during the morphing process.
     */
    lean_vector<hi::glyph_id> glyphs = {};

    /** The position of each of the glyphs.
     */
    lean_vector<hi::aarectangle> glyph_rectangles = {};

    /** The line number where this character is located, counting from top to bottom line.
     */
    size_t line_nr = std::numeric_limits<size_t>::max();

    /** The column number where the character is located on the line, counting from left to right in display order.
     */
    size_t column_nr = std::numeric_limits<size_t>::max();

    /** The unicode description of the grapheme.
     */
    unicode_description const *description = nullptr;

    /** The text direction for this glyph.
     *
     * This is needed to figure out where the location of the insert cursor is compared to the character.
     */
    unicode_bidi_class direction = unicode_bidi_class::unknown;

    /** The script of this character.
     * The script of the character is based on:
     * - The actual script of this unicode character, or if `unicode_script::Common`;
     * - The script of characters before/after this character in the same word, or if `unicode_script::Common`;
     */
    iso_15924 script = {};

    /** Set to true if this glyph is a white space at the end of a line.
     */
    bool is_trailing_white_space = false;

    /** The glyph is the initial glyph.
     *
     * This flag is set to true after loading the initial glyph.
     * This flag is set to false when the glyph is replaced by the bidi-algorithm
     * or glyph-morphing.
     */
    bool glyph_is_initial = false;

    [[nodiscard]] text_shaper_char(hi::character const& character) noexcept :
        character(character),
        bidi_character(character),
        description(&unicode_description::find(character.grapheme()[0]))
    {
    }
};

} // namespace hi::inline v1
