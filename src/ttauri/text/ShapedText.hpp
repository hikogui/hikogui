// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "AttributedGlyphLine.hpp"
#include "gstring.hpp"
#include "../required.hpp"
#include "../alignment.hpp"
#include "../Path.hpp"
#include "../vec.hpp"
#include "../nested_vector_iterator.hpp"
#include <string_view>
#include <optional>

namespace tt {


/** ShapedText represent a piece of text shaped to be displayed.
 */
class ShapedText {
public:
    using iterator = nested_vector_iterator<
        std::vector<AttributedGlyphLine>::const_iterator,
        std::vector<AttributedGlyphLine>::iterator,
        AttributedGlyphLine::iterator>;

    using const_iterator = nested_vector_iterator<
        std::vector<AttributedGlyphLine>::const_iterator,
        std::vector<AttributedGlyphLine>::const_iterator,
        AttributedGlyphLine::const_iterator>;

    Alignment alignment;
    aarect boundingBox;
    float width;
    vec preferredExtent;

private:
    std::vector<AttributedGlyphLine> lines;

public:
    ShapedText() noexcept :
        alignment(Alignment::MiddleCenter), boundingBox(), width(0.0f), preferredExtent(), lines() {}
    ShapedText(ShapedText const &other) = default;
    ShapedText(ShapedText &&other) noexcept = default;
    ShapedText &operator=(ShapedText const &other) = default;
    ShapedText &operator=(ShapedText &&other) noexcept = default;
    ~ShapedText() = default;

    /** Create shaped text from attributed text..
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
     * @param alignment The alignment of the text within the extent.
     * @param width The width into which the text is horizontally aligned.
     * @param wrap True when text should be wrapped to fit inside the given width.
     */
    ShapedText(
        std::vector<AttributedGrapheme> const &text,
        float width,
        Alignment const alignment=Alignment::MiddleCenter,
        bool wrap=true
    ) noexcept;

    /** Create shaped text from a string.
     * This function is mostly used for drawing label text.
     *
     * @param text The text to draw.
     * @param style The text style.
     * @param extent The size of the box to draw in.
     * @param alignment The alignment of the text within the extent.
     * @param wrap When fitting the text in the extent wrap lines when needed.
     */
    ShapedText(
        gstring const &text,
        TextStyle const &style,
        float width,
        Alignment const alignment=Alignment::MiddleCenter,
        bool wrap=true
    ) noexcept;

    /** Create shaped text from a string.
     * This function is mostly used for drawing label text.
     *
     * @param text The text to draw.
     * @param style The text style.
     * @param extent The size of the box to draw in.
     * @param alignment The alignment of the text within the extent.
     * @param wrap When fitting the text in the extent wrap lines when needed.
     */
    ShapedText(
        std::u8string_view text,
        TextStyle const &style,
        float width,
        Alignment const alignment=Alignment::MiddleCenter,
        bool wrap=true
    ) noexcept;

    [[nodiscard]] size_t size() const noexcept {
        ssize_t count = 0;
        for (ttlet &line: lines) {
            count += std::ssize(line);
        }
        return numeric_cast<size_t>(count);
    }

    [[nodiscard]] iterator begin() noexcept { return nested_vector_iterator_begin(lines); }
    [[nodiscard]] const_iterator begin() const noexcept { return nested_vector_iterator_cbegin(lines); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return nested_vector_iterator_cbegin(lines); }

    [[nodiscard]] iterator end() noexcept { return nested_vector_iterator_end(lines); }
    [[nodiscard]] const_iterator end() const noexcept { return nested_vector_iterator_cend(lines); }
    [[nodiscard]] const_iterator cend() const noexcept { return nested_vector_iterator_cend(lines); }

    float topAccender() const noexcept {
        return lines.front().ascender;
    }

    float bottomDescender() const noexcept {
        return lines.back().descender;
    }

    float topCapHeight() const noexcept {
        return lines.front().capHeight;
    }

    float bottomCapHeight() const noexcept {
        return lines.back().capHeight;
    }

    /** Get the capHeight of the middle line(s).
     */
    float middleCapHeight() const noexcept {
        if ((std::ssize(lines) % 2) == 1) {
            return lines[std::ssize(lines) / 2].capHeight;
        } else {
            return (lines[std::ssize(lines) / 2 - 1].capHeight + lines[std::ssize(lines) / 2].capHeight) * 0.5f;
        }
    }

    /** Get the offset of the baseline
     * The offset of the baseline when the text needs to be rendered inside
     * a box of the given height.
     * The offset is depended on the vertical alignment of the shaped text.
     */
    float baselineOffset(float height) noexcept {
        if (alignment == VerticalAlignment::Top) {
            return height - topAccender();
        } else if (alignment == VerticalAlignment::Bottom) {
            return bottomDescender();
        } else if (alignment == VerticalAlignment::Middle) {
            return height * 0.5f - middleCapHeight() * 0.5f;
        } else {
            tt_no_default;
        }
    }

    /** Get the offset of the middle of a line.
    * The offset of the baseline when the middle of a line needs to be
    * at a specific height.
    * The offset is depended on the vertical alignment of the shaped text.
    */
    float middleOffset(float height) const noexcept {
        if (alignment == VerticalAlignment::Top) {
            return height - topCapHeight() * 0.5f;
        } else if (alignment == VerticalAlignment::Bottom) {
            return height - bottomCapHeight() * 0.5f;
        } else if (alignment == VerticalAlignment::Middle) {
            return height - middleCapHeight() * 0.5f;
        } else {
            tt_no_default;
        }
    }

    /** Get the translation for where to place the text.
    * @param rectangle The rectangle where the text should be aligned into.
    *                  The width is ignored and assumed to be the same as the width
    *                  passed during text shaping.
    */
    mat::T2 T(aarect rectangle) noexcept {
        return {
            rectangle.x(),
            rectangle.y() + baselineOffset(rectangle.height())
        };
    }

    /** Get the translation for where to place the text.
    * @param position x is the left position,
    *                 y is where the middle of the line should be.
    */
    mat::T2 TMiddle(vec position) noexcept {
        return {
            position.x(),
            middleOffset(position.y())
        };
    }


    /** Find a glyph that corresponds to position.
     */
    [[nodiscard]] const_iterator find(ssize_t position) const noexcept;

    /** Get a rectangle for the grapheme.
     * The rectangle describes the edges of the grapheme:
     *  - From left side bearing to right side bearing of the glyph.
     *  - from descender to ascender of the line that the glyph is part of.
     *
     * @param index 
     * @return A rectangle describing the position of the grapheme.
     */
    [[nodiscard]] aarect rectangleOfGrapheme(ssize_t index) const noexcept;

    /** Return the cursor-carets.
     * The caret will be to the left of the character at position.
     * 
     * @param index Logical grapheme index.
     * @param overwrite When true display a overwrite cursor.
     * @return left-to-right caret rectangle to display.
     */
    [[nodiscard]] aarect leftToRightCaret(ssize_t index, bool overwrite) const noexcept;

    /** Return a list of merged rectangles to display for the selection.
     * The selection may be discontinues due to bidirectional text.
     *
     * @param first The first logical grapheme that is selected.
     * @param last One beyond the last logical grapheme that is selected.
     * @return A list of rectangles to display.
     */
    [[nodiscard]] std::vector<aarect> selectionRectangles(ssize_t first, ssize_t last) const noexcept;

    /** Return the index of the character .
    * @param logicalIndex The character at logicalIndex.
    * @return logicalIndex of the character to the left.
    */
    [[nodiscard]] std::optional<ssize_t> indexOfCharAtCoordinate(vec coordinate) const noexcept;

    /** Return the index of the character to the left.
     * @param logicalIndex The character at logicalIndex.
     * @return logicalIndex of the character to the left.
     */
    [[nodiscard]] std::optional<ssize_t> indexOfCharOnTheLeft(ssize_t logicalIndex) const noexcept;

    /** Return the index of the character to the right.
    * @param logicalIndex The character at logicalIndex.
    * @return logicalIndex of the character to the right.
    */
    [[nodiscard]] std::optional<ssize_t> indexOfCharOnTheRight(ssize_t logicalIndex) const noexcept;

    /** Return the index at the left side and right side of a word
     */
    [[nodiscard]] std::pair<ssize_t,ssize_t> indicesOfWord(ssize_t logicalIndex) const noexcept;

    /** Return the index at the left side and right side of a paragraph
    */
    [[nodiscard]] std::pair<ssize_t,ssize_t> indicesOfParagraph(ssize_t logicalIndex) const noexcept;

    /** Return the index at the left side of a word
    */
    [[nodiscard]] ssize_t indexAtRightSideOfWord(ssize_t logicalIndex) const noexcept;

    /** Return the index of the word to the left.
    * @param logicalIndex The character at logicalIndex.
    * @return logicalIndex of the character at the start of the word to the left.
    */
    [[nodiscard]] std::optional<ssize_t> indexOfWordOnTheLeft(ssize_t logicalIndex) const noexcept;

    /** Return the index of the word to the right.
    * @param logicalIndex The character at logicalIndex.
    * @return logicalIndex of the character at the start of the next word to the right.
    */
    [[nodiscard]] std::optional<ssize_t> indexOfWordOnTheRight(ssize_t logicalIndex) const noexcept;

    /** Convert the whole shaped text into a layered path.
     */
    [[nodiscard]] Path get_path() const noexcept;


    /** Get the index into the text from a coordinate.
     * The index returned is from the text that was used to construct the ShapedText.
     *
     * @param coordinate A coordinate within the box.
     * @return index of the selected grapheme, or -1 if no grapheme was found near the coordinate.
     */
    [[nodiscard]] int indexFromCoordinate(vec coordinate) const noexcept;

    /** Get the index into the text from a coordinate.
     * The index returned is from the text that was used to construct the ShapedText.
     *
     * @param start The coordinate at the start of a mouse drag.
     * @param current The current coordinate of the mouse pointer, during the drag.
     * @return indices of all the graphemes selected during a drag.
     */
    [[nodiscard]] std::vector<int> indicesFromCoordinates(vec start, vec current) const noexcept;
};



}
