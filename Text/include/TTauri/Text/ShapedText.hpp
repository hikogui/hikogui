// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/AttributedGlyph.hpp"
#include "TTauri/Text/gstring.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/vec.hpp"
#include <string_view>

namespace TTauri::Text {

/** ShapedText represent a piece of text shaped to be displayed.
 */
class ShapedText {
    vec extent;
    Alignment alignment;
    bool wrap;

    std::vector<AttributedGlyph> text;
    vec text_extent;

public:
    ShapedText() noexcept :
        extent(0.0f, 0.0f), alignment(Alignment::BaseCenter), wrap(true), text(), text_extent(0.0f, 0.0f) {}
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
        vec const extent,
        Alignment const alignment=Alignment::BaseCenter,
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
        vec const extent,
        Alignment const alignment=Alignment::BaseCenter,
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
        std::string const &text,
        TextStyle const &style,
        vec const extent,
        Alignment const alignment=Alignment::BaseCenter,
        bool wrap=true
    ) noexcept;

    [[nodiscard]] std::vector<AttributedGlyph>::const_iterator begin() const noexcept {
        return text.cbegin();
    }

    [[nodiscard]] std::vector<AttributedGlyph>::const_iterator end() const noexcept {
        return text.cend();
    }

    /** Return the cursor-carets.
     * For left-to-right caret:
     *   - The position is the right side of the character on the left.
     *   - If position is at the start of a line:
     *     - The character on the right is used.
     *     - 
     * 
     * @param position Logical grapheme position.
     * @return left-to-right caret position, right-to-left caret position.
     *         Both values are the same when there is only a single caret.
     */
    [[nodiscard]] std::pair<vec,vec> carets(ssize_t position) const noexcept;

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
