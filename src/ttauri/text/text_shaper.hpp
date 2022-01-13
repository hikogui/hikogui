// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_shaper_char.hpp"
#include "text_shaper_line.hpp"
#include "glyph_metrics.hpp"
#include "font_metrics.hpp"
#include "grapheme.hpp"
#include "gstring.hpp"
#include "text_style.hpp"
#include "glyph_ids.hpp"
#include "unicode_description.hpp"
#include "font.hpp"
#include "../alignment.hpp"
#include "../coroutine.hpp"
#include <vector>
#include <tuple>

namespace tt::inline v1 {
class font_book;

/** Text shaper.
 *
 * This class takes text as a set of graphemes attributed with font, size, style and color.
 *
 * Steps:
 *  1. Load default glyphs and metrics scaled to the font-size of each glyph.
 *  2. Fold default glyphs to a certain width by inserting line-separators.
 *  3. Run unicode bidirectional algorithm.
 *  4. Reload glyphs and metrics of any brackets.
 *  5. Morph glyphs.
 *  6. Position glyphs including kerning and justification.
 *
 */
class text_shaper {
public:
    using char_vector = std::vector<text_shaper_char>;
    using char_iterator = char_vector::iterator;
    using char_const_iterator = char_vector::const_iterator;
    using line_vector = std::vector<text_shaper_line>;
    using line_iterator = line_vector::iterator;
    using line_const_iterator = line_vector::const_iterator;

    constexpr text_shaper() noexcept = default;
    constexpr text_shaper(text_shaper const &) noexcept = default;
    constexpr text_shaper(text_shaper &&) noexcept = default;
    constexpr text_shaper &operator=(text_shaper const &) noexcept = default;
    constexpr text_shaper &operator=(text_shaper &&) noexcept = default;

    /** Construct a text_shaper with a text and alignment.
     *
     * The constructor will load all the default glyphs for the text.
     *
     * Horizontal alignment is done for each line independent of the writing direction.
     * This allows labels to remain aligned in the same direction on the user-interface
     * even when the labels have translations in different languages.
     *
     * Label widgets should flip the alignment passed to the text shaper when the
     * user interface is mirrored.
     *
     * Text edit fields may want to change the alignment of the text depending on the
     * dominant writing direction, for more natural typing.
     *
     * Vertical alignment of the text determines what y=0 means:
     *  - top: y = 0 is the base-line of the first line, all other lines are at y < 0.
     *  - bottom: y = 0 is the base-line of the last line, all other lines are at y > 0.
     *  - middle, odd: y = 0 is the base-line of the middle line.
     *  - middle, even: y = 0 is half way between the base-lines of the middle two lines.
     *
     * @param text The text as a vector of attributed graphemes.
     *             Use U+2029 as paragraph separator, and if needed U+2028 as line separator.
     * @param vertical_alignment How the text will be aligned vertically.
     * @param line_spacing A multiplier to scale the distance between lines compared to the
     *                     natural line spacing of the font: ascender + descender + line-gap.
     * @param paragraph_spacing A multiplier to scale the distance between lines compared to the
     *                          line spacing after @a line_spacing argument has been applied.
     */
    [[nodiscard]] text_shaper(tt::font_book &font_book, gstring const &text, text_style const &style) noexcept;

    [[nodiscard]] text_shaper(font_book &font_book, std::string_view text, text_style const &style) noexcept;

    [[nodiscard]] char_iterator begin() noexcept
    {
        return _text.begin();
    }

    [[nodiscard]] char_const_iterator begin() const noexcept
    {
        return _text.begin();
    }

    [[nodiscard]] char_const_iterator cbegin() const noexcept
    {
        return _text.cbegin();
    }

    [[nodiscard]] char_iterator end() noexcept
    {
        return _text.end();
    }

    [[nodiscard]] char_const_iterator end() const noexcept
    {
        return _text.end();
    }

    [[nodiscard]] char_const_iterator cend() const noexcept
    {
        return _text.cend();
    }

    /** Get bounding rectangle.
     *
     * It will estimate the width and height based on the glyphs before glyph-morphing and kerning
     * and fold the lines using the unicode line breaking algorithm to the @a max_line_width.
     *
     * The @a alignment parameter is used to align the lines vertically:
     *  - top: y=0 is the base-line of the top line, with following lines below it.
     *  - bottom: y=0 is the base-line of the bottom line, with previous lines above it.
     *  - middle, odd number of lines: y=0 is the base-line of the middle line.
     *  - middle, even number of lines: y=0 is half-way between the base-line of the two lines in the middle.
     *
     * @param maximum_line_width The maximum line width allowed, this may be infinite to determine
     *        the natural text size without folding.
     * @param alignment The vertical alignment of text.
     * @param line_spacing The scaling of the spacing between lines.
     * @param paragraph_spacing The scaling of the spacing between paragraphs.
     * @return The rectangle surrounding the text, cap-height. The rectangle excludes ascenders & descenders, as if
     *         each line is x-height. y = 0 of the rectangle is at the base-line of the text. The returned cap-height is for the
     *         line which is at y = 0.
     */
    [[nodiscard]] std::pair<aarectangle, float> bounding_rectangle(
        float maximum_line_width,
        vertical_alignment alignment,
        float line_spacing = 1.0f,
        float paragraph_spacing = 1.5f) const noexcept;

    /** Layout the lines of the text.
     *
     * It will estimate the width and height based on the glyphs before glyph-morphing and kerning
     * and fold the lines using the unicode line breaking algorithm to the @a max_line_width.
     *
     * The @a alignment parameter is used to align the lines vertically:
     *  - top: y=0 is the base-line of the top line, with following lines below it.
     *  - bottom: y=0 is the base-line of the bottom line, with previous lines above it.
     *  - middle, odd number of lines: y=0 is the base-line of the middle line.
     *  - middle, even number of lines: y=0 is half-way between the base-line of the two lines in the middle.
     *
     * @post The lines have been laid out.
     * @param rectangle The rectangle to position the glyphs in.
     * @param base_line The position of the recommended base-line.
     * @param sub_pixel_size The size of a sub-pixel in device-independent-pixels.
     * @param writing_direction The default writing direction.
     * @param alignment The alignment of the text (default: flush, middle).
     * @param line_spacing The scaling of the spacing between lines (default: 1.0).
     * @param paragraph_spacing The scaling of the spacing between paragraphs (default: 1.5).
     */
    [[nodiscard]] void layout(
        aarectangle rectangle,
        float base_line,
        extent2 sub_pixel_size,
        unicode_bidi_class writing_direction,
        tt::alignment alignment = tt::alignment{horizontal_alignment::flush, vertical_alignment::middle},
        float line_spacing = 1.0f,
        float paragraph_spacing = 1.5f) noexcept;

    /** find the nearest character.
     *
     * @param point The point near
     * @return The index to the character that is nearest to the point.
     */
    [[nodiscard]] ssize_t get_nearest(point2 point) const noexcept;

    /** Get the character to the left.
     *
     * @param index The index to a character.
     * @return The index to the character on the left,
     *         or the most right character in the line above,
     *         or -1 if before begin of text.
     */
    [[nodiscard]] ssize_t left_of(ssize_t index) const noexcept;

    /** Get the character to the right.
     *
     * @param index The index to a character.
     * @return The index to the character on the right,
     *         or the most left character in the line below,
     *         or one beyond the end of text.
     */
    [[nodiscard]] ssize_t right_of(ssize_t index) const noexcept;

    /** Get the character above.
     *
     * @param index The index to a character.
     * @return The index to the character above,
     *         or -1 if before begin of text.
     */
    [[nodiscard]] ssize_t above(ssize_t index) const noexcept;

    /** Get the character to the right.
     *
     * @param index The index to a character.
     * @return The index to the character below,
     *         or one beyond the end of text.
     */
    [[nodiscard]] ssize_t below(ssize_t index) const noexcept;

private:
    font_book *_font_book = nullptr;

    /** A list of character in logical order.
     *
     * @note Graphemes are not allowed to be typographical-ligatures.
     * @note line-feeds, carriage-returns & form-feeds must be replaced by paragraph-separators or line-separators.
     * @note This variable is marked mutable because make_lines() has to modify the characters in the text.
     */
    char_vector _text;

    /** A list of lines top-to-bottom order.
     *
     * The characters contained in each line are in display order.
     */
    line_vector _lines;

    /** Create lines from the characters in the text shaper.
     *
     * @param rectangle The rectangle to position the glyphs in.
     * @param base_line The position of the recommended base-line.
     * @param sub_pixel_size The size of a sub-pixel in device-independent-pixels.
     * @param vertical_alignment The vertical alignment of text (default: middle).
     * @param line_spacing The scaling of the spacing between lines (default: 1.0).
     * @param paragraph_spacing The scaling of the spacing between paragraphs (default: 1.5).
     */
    [[nodiscard]] line_vector make_lines(
        aarectangle rectangle,
        float base_line,
        extent2 sub_pixel_size,
        tt::vertical_alignment vertical_alignment,
        float line_spacing,
        float paragraph_spacing) const noexcept;

    /** Position the glyphs.
     *
     * @param rectangle The rectangle to position the glyphs in.
     * @param sub_pixel_size The size of a sub-pixel in device-independent-pixels.
     * @param horizontal_alignment The horizontal alignment of the text (default: flush).
     * @param writing_direction The default writing direction.
     * @post Glyphs in _text are positioned inside the given rectangle.
     */
    void position_glyphs(
        aarectangle rectangle,
        extent2 sub_pixel_size,
        tt::horizontal_alignment horizontal_alignment,
        unicode_bidi_class writing_direction) noexcept;

    /** Get column and line of a character.
     */
    //[[nodiscard]] std::pair<ssize_t, ssize_t> text_shaper::get_column_line(ssize_t index) const noexcept;
};

} // namespace tt::inline v1
