// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_style.hpp"
#include "glyph_ids.hpp"
#include "glyph_metrics.hpp"
#include "font.hpp"
#include "../unicode/unicode_description.hpp"
#include "../unicode/grapheme.hpp"
#include "../geometry/point.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"

namespace tt::inline v1 {
class font_book;

class text_shaper_char {
public:
    /** The grapheme.
     */
    tt::grapheme grapheme;

    /** The style of how to display the grapheme.
     */
    tt::text_style style;

    /** The scale to resize the font's size to match the physical display.
     */
    float dpi_scale = 1.0f;

    /** The glyph representing one or more graphemes.
     * The glyph will change during shaping of the text:
     *  1. The initial glyph, used for determining the width of the grapheme
     *     and the folding algorithm.
     *  2. The glyph representing a bracket may be replaced with a mirrored bracket
     *     by the bidi-algorithm.
     *  3. The glyph may be replaced by the font using the glyph-morphing algorithms
     *     for better continuation of cursive text and merging of graphemes into
     *     a ligature.
     */
    tt::glyph_ids glyph;

    /** The glyph metrics of the currently glyph.
     *
     * The metrics are scaled by `scale`.
     */
    tt::glyph_metrics metrics;

    /** The line number where this character is located, counting from top to bottom line.
     */
    size_t line_nr;

    /** The column number where the character is located on the line, counting from left to right in display order.
     */
    size_t column_nr;

    /** Position of the character.
     *
     * For a non-ligature this is the origin of the glyph, where the actual glyph
     * is located at `position + metrics.bounding_rectangle`.
     * For ligatures the position is moved based on the advance of each character within the ligature.
     */
    point2 position;

    /** The rectangle for this character.
     *
     * The rectangle is used for:
     *  - creating a selection box around the character.
     *  - creating cursors before, after and on the character.
     *  - converting mouse-position to character.
     *
     * The attributes of the rectangle are:
     *  - left side is equal to the position.x
     *  - The width is the advance of the character within the ligature.
     *    Or if the glyph is not a ligature the width is the same as the advance.
     *  - The bottom is at the descender
     *  - The top is at the ascender
     *
     * When multiple characters are converted to a ligature, the
     * rectangle of each of those characters occupies a
     * subsection of the ligature-glyph. In this case the left most
     * character will contain the ligature-glyph, and the rest of
     * the characters of the ligature will have empty glyphs.
     */
    aarectangle rectangle;

    /** The unicode description of the grapheme.
     */
    unicode_description const *description;

    /** The text direction for this glyph.
     *
     * This is needed to figure out where the location of the insert cursor is compared to the character.
     */
    unicode_bidi_class direction;

    /** The script of this character.
     * The script of the character is based on:
     * - The actual script of this unicode character, or if `unicode_script::Common`;
     * - The script of characters before/after this character in the same word, or if `unicode_script::Common`;
     * - The script passed during construction of the text_shaper.
     */
    unicode_script script;

    /** The scale of the glyph for displaying on the screen.
     */
    float scale = 1.0f;

    /** The width used for this grapheme when folding lines.
     *
     * This width is based on the initial glyph's advance after converting the grapheme
     * using the text-style into a glyph. This width excludes kerning and glyph-morphing.
     */
    float width = 0.0f;

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

    [[nodiscard]] text_shaper_char(tt::grapheme const &grapheme, text_style const &style, float dpi_scale) noexcept;

    /** Initialize the glyph based on the grapheme.
     *
     * @note The glyph is only initialized when `glyph_is_initial == false`.
     * @post `glyph`, `metrics` and `width` are modified. `glyph_is_initial` is set to true.
     */
    void initialize_glyph(tt::font_book &font_book, tt::font const &font) noexcept;

    /** Initialize the glyph based on the grapheme.
     *
     * @note The glyph is only initialized when `glyph_is_initial == false`.
     * @post `glyph`, `metrics` and `width` are modified. `glyph_is_initial` is set to true.
     */
    void initialize_glyph(tt::font_book &font_book) noexcept;

    /** Called by the bidi-algorithm to mirror glyphs.
     *
     * The glyph is replaced with a glyph from the same font using the given code-point.
     *
     * @pre `glyph.num_grapheme == 1`.
     * @post `glyph` and `metrics` are modified. `glyph_is_initial` is set to false.
     * @note The `width` remains based on the original glyph.
     */
    void replace_glyph(char32_t code_point) noexcept;

    /** Get the scaled font metrics for this character.
     */
    [[nodiscard]] tt::font_metrics font_metrics() const noexcept
    {
        return scale * glyph.font().metrics;
    }

    [[nodiscard]] vector2 get_kerning(text_shaper_char const &next) const noexcept
    {
        if (&(glyph.font()) != &(next.glyph.font()) or scale != next.scale or not glyph.has_num_glyphs<1>() or
            not next.glyph.has_num_glyphs<1>()) {
            return vector2{};
        } else {
            ttlet kerning = glyph.font().get_kerning(glyph.get_single(), next.glyph.get_single());
            return scale * kerning;
        }
    }

    [[nodiscard]] friend bool operator==(text_shaper_char const &lhs, char32_t const &rhs) noexcept
    {
        return lhs.grapheme == rhs;
    }

    [[nodiscard]] friend bool operator==(text_shaper_char const &lhs, char const &rhs) noexcept
    {
        return lhs.grapheme == rhs;
    }

private:
    /** Load metrics based on the loaded glyph.
     */
    void set_glyph(tt::glyph_ids &&new_glyph) noexcept;
};

} // namespace tt::inline v1
