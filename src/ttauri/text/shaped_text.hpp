// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "attributed_glyph_line.hpp"
#include "gstring.hpp"
#include "../required.hpp"
#include "../alignment.hpp"
#include "../graphic_path.hpp"
#include "../recursive_iterator.hpp"
#include "../geometry/extent.hpp"
#include "../geometry/point.hpp"
#include <string_view>
#include <optional>

namespace tt::inline v1 {
class font_book;

/** shaped_text represent a piece of text shaped to be displayed.
 */
class shaped_text {
public:
    using iterator = recursive_iterator<std::vector<attributed_glyph_line>::iterator>;

    using const_iterator = recursive_iterator<std::vector<attributed_glyph_line>::const_iterator>;

    alignment alignment;
    aarectangle boundingBox;
    float width;

private:
    std::vector<attributed_glyph_line> lines;
    extent2 _preferred_extent;

public:
    shaped_text() noexcept :
        alignment{horizontal_alignment::center, vertical_alignment::middle},
        boundingBox(),
        width(0.0f),
        _preferred_extent(),
        lines()
    {
    }
    shaped_text(shaped_text const &other) = default;
    shaped_text(shaped_text &&other) noexcept = default;
    shaped_text &operator=(shaped_text const &other) = default;
    shaped_text &operator=(shaped_text &&other) noexcept = default;
    ~shaped_text() = default;

    /** Create shaped text from attributed text.
     * This function is used to draw rich-text.
     * Each grapheme comes with its own text-style.
     *
     * Vertical alignment is based on base line at y=0.0:
     *  * Bottom -> base line of the last line is at y=0.0
     *  * Top -> base line of the first line is at y=0.0
     *  * Middle ->
     *    * Odd # lines -> base line of middle line is at y=0.0
     *    * Even # lines -> gap between the two middle lines is at y=0.0
     *
     * Horizontal alignment is based on the given width:
     *  * Left -> first character starts at x=0.0
     *  * Right -> last visible character end at x=maximum_width
     *  * Center -> middle of the visible text at x=maximum_width/2
     *
     * @param text The text to draw.
     * @param width The width into which the text is horizontally aligned.
     * @param alignment The alignment of the text within the extent.
     * @param wrap True when text should be wrapped to fit inside the given width.
     */
    shaped_text(
        tt::font_book const &font_book,
        std::vector<attributed_grapheme> const &text,
        float width,
        tt::alignment const alignment = tt::alignment{horizontal_alignment::center, vertical_alignment::middle},
        bool wrap = true) noexcept;

    /** Create shaped text from a string.
     * This function is mostly used for drawing label text.
     *
     * @param text The text to draw.
     * @param style The text style.
     * @param width The maximum a width of the text.
     * @param alignment The alignment of the text within the extent.
     * @param wrap When fitting the text in the extent wrap lines when needed.
     */
    shaped_text(
        tt::font_book const &font_book,
        gstring const &text,
        text_style const &style,
        float width,
        tt::alignment const alignment = tt::alignment{horizontal_alignment::center, vertical_alignment::middle},
        bool wrap = true) noexcept;

    /** Create shaped text from a string.
     * This function is mostly used for drawing label text.
     *
     * @param text The text to draw.
     * @param style The text style.
     * @param width The maximum width of the text.
     * @param alignment The alignment of the text within the extent.
     * @param wrap When fitting the text in the extent wrap lines when needed.
     */
    shaped_text(
        tt::font_book const &font_book,
        std::string_view text,
        text_style const &style,
        float width,
        tt::alignment const alignment = tt::alignment{horizontal_alignment::center, vertical_alignment::middle},
        bool wrap = true) noexcept;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return lines.empty();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        std::size_t count = 0;
        for (ttlet &line : lines) {
            count += line.size();
        }
        return count;
    }

    [[nodiscard]] extent2 preferred_size() const noexcept
    {
        return _preferred_extent;
    }

    [[nodiscard]] iterator begin() noexcept
    {
        return recursive_iterator_begin(lines);
    }
    [[nodiscard]] const_iterator begin() const noexcept
    {
        return recursive_iterator_begin(lines);
    }
    [[nodiscard]] const_iterator cbegin() const noexcept
    {
        return recursive_iterator_begin(lines);
    }

    [[nodiscard]] iterator end() noexcept
    {
        return recursive_iterator_end(lines);
    }
    [[nodiscard]] const_iterator end() const noexcept
    {
        return recursive_iterator_end(lines);
    }
    [[nodiscard]] const_iterator cend() const noexcept
    {
        return recursive_iterator_end(lines);
    }

    float topAccender() const noexcept
    {
        return lines.front().metrics.ascender;
    }

    float bottomDescender() const noexcept
    {
        return lines.back().metrics.descender;
    }

    float topCapHeight() const noexcept
    {
        return lines.front().metrics.cap_height;
    }

    float bottomCapHeight() const noexcept
    {
        return lines.back().metrics.cap_height;
    }

    /** Get the capHeight of the middle line(s).
     */
    float middleCapHeight() const noexcept
    {
        if (lines.empty()) {
            return 0;
        } else {
            if ((ssize(lines) % 2) == 1) {
                return lines[ssize(lines) / 2].metrics.cap_height;
            } else {
                return (lines[ssize(lines) / 2 - 1].metrics.cap_height + lines[ssize(lines) / 2].metrics.cap_height) * 0.5f;
            }
        }
    }

    /** Get the offset of the baseline
     * The offset of the baseline when the text needs to be rendered inside
     * a box of the given height.
     * The offset is depended on the vertical alignment of the shaped text.
     */
    float baselineOffset(float height) noexcept
    {
        if (alignment == vertical_alignment::top) {
            return height - topAccender();
        } else if (alignment == vertical_alignment::bottom) {
            return bottomDescender();
        } else if (alignment == vertical_alignment::middle) {
            return height * 0.5f - middleCapHeight() * 0.5f;
        } else {
            tt_no_default();
        }
    }

    /** Get the offset of the middle of a line.
     * The offset of the baseline when the middle of a line needs to be
     * at a specific height.
     * The offset is depended on the vertical alignment of the shaped text.
     */
    float middleOffset(float height) const noexcept
    {
        if (alignment == vertical_alignment::top) {
            return height - topCapHeight() * 0.5f;
        } else if (alignment == vertical_alignment::bottom) {
            return height - bottomCapHeight() * 0.5f;
        } else if (alignment == vertical_alignment::middle) {
            return height - middleCapHeight() * 0.5f;
        } else {
            tt_no_default();
        }
    }

    /** Get the translation for where to place the text.
     * @param position x is the left position,
     *                 y is where the middle of the line should be.
     */
    translate2 translate_base_line(point2 position) noexcept
    {
        // Keep the translation integral to have sharper text.
        return translate2{std::round(position.x()), std::round(middleOffset(position.y()))};
    }

    /** Find a glyph that corresponds to position.
     */
    [[nodiscard]] const_iterator find(ssize_t position) const noexcept;

    /** Get a rectangle for the grapheme.
     * The rectangle describes the edges of the grapheme:
     *  - From left side bearing to right side bearing of the glyph.
     *  - from descender to ascender of the line that the glyph is part of.
     *
     * @param index The logical index of the character
     * @return A rectangle describing the position of the grapheme, true if character is left-to-right.
     */
    [[nodiscard]] std::pair<aarectangle, bool> rectangleOfgrapheme(ssize_t index) const noexcept;

    /** Return the cursor-carets.
     * The caret will be to the left of the character at position.
     *
     * @param index Logical grapheme index.
     * @param overwrite When true display a overwrite cursor.
     * @return left-to-right caret rectangle to display.
     */
    [[nodiscard]] aarectangle left_to_right_caret(ssize_t index, bool overwrite) const noexcept;

    /** Return the cursor-carets.
     * The caret will be to the left of the character at position.
     *
     * @param index Logical grapheme index.
     * @param overwrite When true display a overwrite cursor.
     * @return left-to-right caret rectangle to display.
     */
    [[nodiscard]] aarectangle right_to_left_caret(ssize_t index, bool overwrite) const noexcept;

    /** Return a list of merged rectangles to display for the selection.
     * The selection may be discontinues due to bidirectional text.
     *
     * @param first The first logical grapheme that is selected.
     * @param last One beyond the last logical grapheme that is selected.
     * @return A list of rectangles to display.
     */
    [[nodiscard]] std::vector<aarectangle> selection_rectangles(ssize_t first, ssize_t last) const noexcept;

    /** Get the character close to a coordinate.
     * @param coordinate The coordinate of the mouse pointer.
     * @return The logical index of the character closest to the coordinate
     */
    [[nodiscard]] std::optional<ssize_t> index_of_grapheme_at_coordinate(point2 coordinate) const noexcept;

    /** Get the character left of the given character
     * @param logical_index The index of the logical character pointed to.
     * @return The logical index of the character left-closest to the given character.
     */
    [[nodiscard]] std::optional<ssize_t> indexOfCharOnTheLeft(ssize_t logical_index) const noexcept;

    /** Get the character right of the given character
     * @param logical_index The index of the logical character pointed to.
     * @return The logical index of the character right-closest to the given character.
     */
    [[nodiscard]] std::optional<ssize_t> indexOfCharOnTheRight(ssize_t logical_index) const noexcept;

    /** Get the word with the given character
     * @param logical_index The index of the logical character pointed to.
     * @return The logical indices of the first and last character of a word.
     */
    [[nodiscard]] std::pair<ssize_t, ssize_t> indices_of_word(ssize_t logical_index) const noexcept;

    /** Get the character right of the given character
     * @param logical_index The index of the logical character pointed to.
     * @return The logical indices of the first and last character of a paragraph.
     */
    [[nodiscard]] std::pair<ssize_t, ssize_t> indices_of_paragraph(ssize_t logical_index) const noexcept;

    /** Get the character right of the given character
     * @param logical_index The index of the logical character pointed to.
     * @return The logical indices beyond the last character of a word.
     */
    [[nodiscard]] ssize_t indexAtRightSideOfWord(ssize_t logical_index) const noexcept;

    /** Get the first character of the word on the left.
     * @param logical_index The index of the logical character pointed to.
     * @return The logical index of a first letter of the word on the left
     */
    [[nodiscard]] std::optional<ssize_t> indexOfWordOnTheLeft(ssize_t logical_index) const noexcept;

    /** Get the last character of the word on the right.
     * @param logical_index The index of the logical character pointed to.
     * @return The logical index of a last letter of the word on the right
     */
    [[nodiscard]] std::optional<ssize_t> indexOfWordOnTheRight(ssize_t logical_index) const noexcept;

    /** Convert the whole shaped text into a layered path.
     */
    [[nodiscard]] graphic_path get_path() const noexcept;

    /** Get the index into the text from a coordinate.
     * The index returned is from the text that was used to construct the shaped_text.
     *
     * @param coordinate A coordinate within the box.
     * @return index of the selected grapheme, or -1 if no grapheme was found near the coordinate.
     */
    [[nodiscard]] int indexFromCoordinate(point2 coordinate) const noexcept;

    /** Get the index into the text from a coordinate.
     * The index returned is from the text that was used to construct the shaped_text.
     *
     * @param start The coordinate at the start of a mouse drag.
     * @param current The current coordinate of the mouse pointer, during the drag.
     * @return indices of all the graphemes selected during a drag.
     */
    [[nodiscard]] std::vector<int> indicesFromCoordinates(point2 start, point2 current) const noexcept;
};

} // namespace tt::inline v1
