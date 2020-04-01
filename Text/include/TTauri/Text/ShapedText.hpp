// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/AttributedGlyphLine.hpp"
#include "TTauri/Text/gstring.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/nested_vector_iterator.hpp"
#include <string_view>
#include <optional>

namespace TTauri::Text {


/** ShapedText represent a piece of text shaped to be displayed.
 */
class ShapedText {
    using iterator = nested_vector_iterator<
        std::vector<AttributedGlyphLine>::const_iterator,
        std::vector<AttributedGlyphLine>::iterator,
        AttributedGlyphLine::iterator>;

    using const_iterator = nested_vector_iterator<
        std::vector<AttributedGlyphLine>::const_iterator,
        std::vector<AttributedGlyphLine>::const_iterator,
        AttributedGlyphLine::const_iterator>;

    vec extent;
    std::vector<AttributedGlyphLine> lines;

public:
    ShapedText() noexcept :
        extent(0.0f, 0.0f), lines() {}
    ShapedText(ShapedText const &other) = default;
    ShapedText(ShapedText &&other) noexcept = default;
    ShapedText &operator=(ShapedText const &other) = default;
    ShapedText &operator=(ShapedText &&other) noexcept = default;
    ~ShapedText() = default;

    /** Create shaped text from attributed text..
     * This function is used to draw rich-text.
     * Each grapheme comes with its own text-style.
     *
     * @param text The text to draw.
     * @param extent The size of the box to draw in.
     * @param alignment The alignment of the text within the extent.
     * @param wrap When fitting the text in the extent wrap lines when needed.
     */
    ShapedText(
        std::vector<AttributedGrapheme> const &text,
        HorizontalAlignment const alignment=HorizontalAlignment::Center,
        float maximum_width=std::numeric_limits<float>::max()
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
        HorizontalAlignment const alignment=HorizontalAlignment::Center,
        float maximum_width=std::numeric_limits<float>::max()
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
        std::string const &text,
        TextStyle const &style,
        HorizontalAlignment const alignment=HorizontalAlignment::Center,
        float maximum_width=std::numeric_limits<float>::max()
    ) noexcept;

    [[nodiscard]] size_t size() const noexcept {
        ssize_t count = 0;
        for (let &line: lines) {
            count += ssize(line);
        }
        return numeric_cast<size_t>(count);
    }

    [[nodiscard]] iterator begin() noexcept { return nested_vector_iterator_begin(lines); }
    [[nodiscard]] const_iterator begin() const noexcept { return nested_vector_iterator_cbegin(lines); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return nested_vector_iterator_cbegin(lines); }

    [[nodiscard]] iterator end() noexcept { return nested_vector_iterator_end(lines); }
    [[nodiscard]] const_iterator end() const noexcept { return nested_vector_iterator_cend(lines); }
    [[nodiscard]] const_iterator cend() const noexcept { return nested_vector_iterator_cend(lines); }

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
    [[nodiscard]] rect rectangleOfGrapheme(ssize_t index) const noexcept;

    /** Return the cursor-carets.
     * The caret will be to the left of the character at position.
     * 
     * @param index Logical grapheme index.
     * @param overwrite When true display a overwrite cursor.
     * @return left-to-right caret rectangle to display.
     */
    [[nodiscard]] rect leftToRightCaret(ssize_t index, bool overwrite) const noexcept;

    /** Return a list of merged rectangles to display for the selection.
     * The selection may be discontinues due to bidirectional text.
     *
     * @param first The first logical grapheme that is selected.
     * @param last One beyond the last logical grapheme that is selected.
     * @return A list of rectangles to display.
     */
    [[nodiscard]] std::vector<rect> selectionRectangles(ssize_t first, ssize_t last) const noexcept;

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
