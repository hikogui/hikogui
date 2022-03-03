// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_shaper_char.hpp"
#include "text_shaper_line.hpp"
#include "text_cursor.hpp"
#include "glyph_metrics.hpp"
#include "font_metrics.hpp"
#include "text_style.hpp"
#include "glyph_ids.hpp"
#include "font.hpp"
#include "../unicode/unicode_description.hpp"
#include "../unicode/unicode_break_opportunity.hpp"
#include "../unicode/grapheme.hpp"
#include "../unicode/gstring.hpp"
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
     * @param dpi_scale The scaling factor to use to scale a font's size to match the physical display.
     * @param script The script of the text.
     */
    [[nodiscard]] text_shaper(
        tt::font_book &font_book,
        gstring const &text,
        text_style const &style,
        float dpi_scale,
        unicode_script script = unicode_script::Common) noexcept;

    [[nodiscard]] text_shaper(
        tt::font_book &font_book,
        std::string_view text,
        text_style const &style,
        float dpi_scale,
        unicode_script script = unicode_script::Common) noexcept;

    [[nodiscard]] bool empty() const noexcept
    {
        return _text.empty();
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return _text.size();
    }

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

    auto const &lines() const noexcept
    {
        return _lines;
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
        float paragraph_spacing = 1.5f) noexcept;

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

    /** The rectangle used when laying out the text.
     */
    [[nodiscard]] aarectangle rectangle() const noexcept
    {
        return _rectangle;
    }

    /** Get the character at index in logical order.
     *
     * @note This function checks for underflow and overflow of index and always returns an iterator
     *       between `begin()` and `end()` inclusive.
     * @param index The index in the text.
     * @return Iterator to the character.
     */
    [[nodiscard]] char_const_iterator get_it(size_t index) const noexcept;

    /** Get the character at the cursor.
     *
     * @note This function checks for underflow and overflow of cursor and always returns an iterator
     *       between `begin()` and `end()` inclusive.
     * @param cursor The cursor in the text.
     * @return Iterator to the character.
     */
    [[nodiscard]] char_const_iterator get_it(text_cursor cursor) const noexcept
    {
        return get_it(cursor.index());
    }

    /** Get the character at column and row in display order.
     *
     * @note This function checks for underflow and overflow of column and row and always returns an iterator
     *       between `begin()` and `end()` inclusive.
     * @param column The column
     * @param row The row
     * @return Iterator to the character.
     */
    [[nodiscard]] char_const_iterator get_it(size_t column, size_t row) const noexcept;

    /** Get the character at column and row in display order.
     *
     * @note This function checks for underflow and overflow of column and row and always returns an iterator
     *       between `begin()` and `end()` inclusive.
     * @param column_row The column, row packed in a `std::pair`.
     * @return Iterator to the character.
     */
    [[nodiscard]] char_const_iterator get_it(std::pair<size_t, size_t> column_row) const noexcept
    {
        return get_it(column_row.first, column_row.second);
    }

    /** Get the column and line of a character.
     *
     * @param it The iterator to the character, or `end()`.
     * @return The (column, row) packed in a `std::pair`.
     */
    [[nodiscard]] std::pair<size_t, size_t> get_column_line(text_shaper::char_const_iterator it) const noexcept;

    /** Get the column and line of a character.
     *
     * @param index The index of the character in logical order.
     * @return The (column, row) packed in a `std::pair`.
     */
    [[nodiscard]] std::pair<size_t, size_t> get_column_line(size_t index) const noexcept
    {
        return get_column_line(get_it(index));
    }

    /** Get the column and line of a character.
     *
     * @param cursor The cursor to the character.
     * @return The (column, row) packed in a `std::pair`.
     */
    [[nodiscard]] std::pair<size_t, size_t> get_column_line(text_cursor cursor) const noexcept
    {
        return get_column_line(cursor.index());
    }

    /** Get the index of the character in logical order.
     *
     * @param it The iterator to the character or `end()`.
     * @return The index in logical order.
     */
    [[nodiscard]] size_t get_index(text_shaper::char_const_iterator it) const noexcept;

    /** Get the cursor at the beginning of the document.
     *
     * @return The cursor at the beginning of the document.
     */
    [[nodiscard]] text_cursor get_begin_cursor() const noexcept;

    /** Get the cursor at the end of the document.
     *
     * @return The cursor at the end of the document.
     */
    [[nodiscard]] text_cursor get_end_cursor() const noexcept;

    /** Get the cursor before the character in logical order.
     *
     * @param it The iterator to the character or `end()`.
     * @return A cursor before the character in logical order.
     */
    [[nodiscard]] text_cursor get_before_cursor(size_t index) const noexcept;

    /** Get the cursor after the character in logical order.
     *
     * @param it The iterator to the character or `end()`.
     * @return A cursor after the character in logical order.
     */
    [[nodiscard]] text_cursor get_after_cursor(size_t index) const noexcept;

    /** Get the cursor before the character in logical order.
     *
     * @param it The iterator to the character or `end()`.
     * @return A cursor before the character in logical order.
     */
    [[nodiscard]] text_cursor get_before_cursor(text_shaper::char_const_iterator it) const noexcept
    {
        return get_before_cursor(get_index(it));
    }

    /** Get the cursor after the character in logical order.
     *
     * @param it The iterator to the character or `end()`.
     * @return A cursor after the character in logical order.
     */
    [[nodiscard]] text_cursor get_after_cursor(text_shaper::char_const_iterator it) const noexcept
    {
        return get_after_cursor(get_index(it));
    }

    /** Get the cursor left of the character in display order.
     *
     * @param it The iterator to the character or `end()`.
     * @return A cursor left of the character in display order.
     */
    [[nodiscard]] text_cursor get_left_cursor(text_shaper::char_const_iterator it) const noexcept;

    /** Get the cursor right of the character in display order.
     *
     * @param it The iterator to the character or `end()`.
     * @return A cursor right of the character in display order.
     */
    [[nodiscard]] text_cursor get_right_cursor(text_shaper::char_const_iterator it) const noexcept;

    /** Check if the cursor is on the left side of the character in display order.
     *
     * @param cursor The cursor to query.
     * @return True if the cursor is on the left of the character.
     */
    [[nodiscard]] bool is_on_left(text_cursor cursor) const noexcept;

    /** Check if the cursor is on the right side of the character in display order.
     *
     * @param cursor The cursor to query.
     * @return True if the cursor is on the right of the character.
     */
    [[nodiscard]] bool is_on_right(text_cursor cursor) const noexcept;

    /** find the nearest character.
     *
     * @param point The point near
     * @return The text_cursor nearest to the point.
     */
    [[nodiscard]] text_cursor get_nearest_cursor(point2 point) const noexcept;

    /** Get the selection for the character at the cursor.
     */
    [[nodiscard]] std::pair<text_cursor, text_cursor> select_char(text_cursor cursor) const noexcept;

    /** Get the selection for the word at the cursor.
     */
    [[nodiscard]] std::pair<text_cursor, text_cursor> select_word(text_cursor cursor) const noexcept;

    /** Get the selection for the sentence at the cursor.
     */
    [[nodiscard]] std::pair<text_cursor, text_cursor> select_sentence(text_cursor cursor) const noexcept;

    /** Get the selection for a paragraph at the cursor.
     */
    [[nodiscard]] std::pair<text_cursor, text_cursor> select_paragraph(text_cursor cursor) const noexcept;

    /** Get the selection for a paragraph at the cursor.
     */
    [[nodiscard]] std::pair<text_cursor, text_cursor> select_document(text_cursor cursor) const noexcept;

    /** Get the character to the left.
     *
     * @param it The iterator to the character.
     * @return The iterator to the character on the left, or empty.
     */
    [[nodiscard]] char_const_iterator move_left_char(char_const_iterator it) const noexcept;

    /** Get the character to the right.
     *
     * @param it The iterator to the character.
     * @return The iterator to the character on the left, or empty.
     */
    [[nodiscard]] char_const_iterator move_right_char(char_const_iterator it) const noexcept;

    [[nodiscard]] text_cursor move_left_char(text_cursor cursor, bool overwrite_mode) const noexcept;
    [[nodiscard]] text_cursor move_right_char(text_cursor cursor, bool overwrite_mode) const noexcept;
    [[nodiscard]] text_cursor move_down_char(text_cursor cursor, float &x) const noexcept;
    [[nodiscard]] text_cursor move_up_char(text_cursor cursor, float &x) const noexcept;
    [[nodiscard]] text_cursor move_left_word(text_cursor cursor, bool overwrite_mode) const noexcept;
    [[nodiscard]] text_cursor move_right_word(text_cursor cursor, bool overwrite_mode) const noexcept;
    [[nodiscard]] text_cursor move_begin_line(text_cursor cursor) const noexcept;
    [[nodiscard]] text_cursor move_end_line(text_cursor cursor) const noexcept;
    [[nodiscard]] text_cursor move_begin_sentence(text_cursor cursor) const noexcept;
    [[nodiscard]] text_cursor move_end_sentence(text_cursor cursor) const noexcept;
    [[nodiscard]] text_cursor move_begin_paragraph(text_cursor cursor) const noexcept;
    [[nodiscard]] text_cursor move_end_paragraph(text_cursor cursor) const noexcept;
    [[nodiscard]] text_cursor move_begin_document(text_cursor cursor) const noexcept;
    [[nodiscard]] text_cursor move_end_document(text_cursor cursor) const noexcept;

private:
    font_book *_font_book = nullptr;

    /** The scaling factor to use to scale a font's size to match the physical pixels on the display.
     */
    float _dpi_scale;

    /** A list of character in logical order.
     *
     * @note Graphemes are not allowed to be typographical-ligatures.
     * @note line-feeds, carriage-returns & form-feeds must be replaced by paragraph-separators or line-separators.
     * @note This variable is marked mutable because make_lines() has to modify the characters in the text.
     */
    char_vector _text;

    /** A list of word break opportunities.
     */
    unicode_break_vector _line_break_opportunities;

    /** A list of widths, one for each character in _text.
     */
    std::vector<float> _line_break_widths;

    /** A list of word break opportunities.
     */
    unicode_break_vector _word_break_opportunities;

    /** A list of sentence break opportunities.
     */
    unicode_break_vector _sentence_break_opportunities;

    /** The default script of the text.
     */
    unicode_script _script;

    /** A list of lines top-to-bottom order.
     *
     * The characters contained in each line are in display order.
     */
    line_vector _lines;

    /** The font metrics of a line without text.
     */
    font_metrics _initial_line_metrics;

    /** The rectangle used for laying out.
     */
    aarectangle _rectangle;

    /** Create lines from the characters in the text shaper.
     *
     * @param rectangle The rectangle to position the glyphs in.
     * @param base_line The position of the recommended base-line.
     * @param sub_pixel_size The size of a sub-pixel in device-independent-pixels.
     * @param vertical_alignment The vertical alignment of text.
     * @param writing_direction The default writing direction.
     * @param line_spacing The scaling of the spacing between lines.
     * @param paragraph_spacing The scaling of the spacing between paragraphs.
     */
    [[nodiscard]] line_vector make_lines(
        aarectangle rectangle,
        float base_line,
        extent2 sub_pixel_size,
        tt::vertical_alignment vertical_alignment,
        unicode_bidi_class writing_direction,
        float line_spacing,
        float paragraph_spacing) noexcept;

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

    /** Resolve the script of each character in text.
     */
    void resolve_script() noexcept;

    [[nodiscard]] std::pair<text_cursor, text_cursor>
    get_selection_from_break(text_cursor cursor, unicode_break_vector const &break_opportunities) const noexcept;
};

} // namespace tt::inline v1
