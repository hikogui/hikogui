// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/AttributedGlyph.hpp"
#include "TTauri/Text/gstring.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/Path.hpp"
#include <string_view>

namespace TTauri::Text {




/** ShapedText represent a piece of text shaped to be displayed.
 */
class ShapedText {
    std::vector<AttributedGlyph> text;
    extent2 box_size;

public:
    ShapedText() noexcept :
        text(), box_size(0.0f, 0.0f) {}

    /** Create shaped text from attributed text..
     * This function is used to draw rich-text.
     * Each grapheme comes with its own text-style.
     *
     * @param text Text to draw.
     * @param alignment Alignment within a box of minimum_size.
     * @param minimum_size The minimum size of the box.
     * @param maximum_size Size that the box may grow to.
     */
    ShapedText(std::vector<AttributedGrapheme> const &text, Alignment const &alignment, extent2 const &minimum_size, extent2 const &maximum_size) noexcept;

    /** Create shaped text from a string.
     * This function is mostly used for drawing label text.
     *
     * @param text Text to draw.
     * @param style Text style
     * @param alignment Alignment within a box of minimum_size.
     * @param minimum_size The minimum size of the box.
     * @param maximum_size Size that the box may grow to.
     */
    ShapedText(gstring const &text, TextStyle const &style, Alignment const &alignment, extent2 const &minimum_size, extent2 const &maximum_size) noexcept;

    /** Create shaped text from a string.
     * This function is mostly used for drawing label text.
     *
     * @param text Text to draw.
     * @param style Text style
     * @param alignment Alignment within a box of minimum_size.
     * @param minimum_size The minimum size of the box.
     * @param maximum_size Size that the box may grow to.
     */
    ShapedText(std::string const &text, TextStyle const &style, Alignment const &alignment, extent2 const &minimum_size, extent2 const &maximum_size) noexcept;

    ShapedText(ShapedText const &other) noexcept = default;
    ShapedText(ShapedText &&other) noexcept = default;
    ShapedText &operator=(ShapedText const &other) noexcept = default;
    ShapedText &operator=(ShapedText &&other) noexcept = default;
    ~ShapedText() = default;

    /** Return the size of the box.
     * The box is at least minimum_size, up to maximum_size.
     */
    [[nodiscard]] extent2 size() const noexcept {
        return box_size;
    }

    /** Convert the whole shaped text into a layered path.
     */
    [[nodiscard]] Path toPath() const noexcept;

    /** Get the index into the text from a coordinate.
     * The index returned is from the text that was used to construct the ShapedText.
     *
     * @param coordinate A coordinate within the box.
     * @return index of the selected grapheme, or -1 if no grapheme was found near the coordinate.
     */
    [[nodiscard]] int indexFromCoordinate(glm::vec2 coordinate) const noexcept;

    /** Get the index into the text from a coordinate.
     * The index returned is from the text that was used to construct the ShapedText.
     *
     * @param start The coordinate at the start of a mouse drag.
     * @param current The current coordinate of the mouse pointer, during the drag.
     * @return indices of all the graphemes selected during a drag.
     */
    [[nodiscard]] std::vector<int> indicesFromCoordinates(glm::vec2 start, glm::vec2 current) const noexcept;
};



}
