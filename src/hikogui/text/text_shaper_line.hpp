// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_shaper_char.hpp"
#include "../font/module.hpp"
#include "../unicode/unicode_bidi_class.hpp"
#include "../geometry/module.hpp"
#include <vector>

namespace hi::inline v1 {

class text_shaper_line {
public:
    using iterator = std::vector<text_shaper_char>::iterator;
    using const_iterator = std::vector<text_shaper_char>::const_iterator;
    using column_vector = std::vector<iterator>;

    /** The first character in the line, in logical order.
     */
    iterator first;

    /** One beyond the last character in the line, in logical order.
     */
    iterator last;

    /** Iterators to the characters in the text.
     *
     * The Iterators are in display-order.
     */
    column_vector columns;

    /** The maximum metrics of the font of each glyph on this line.
     */
    font_metrics metrics;

    /** The line number of this line, counted from top to bottom.
     */
    size_t line_nr;

    /** Position of the base-line of this line.
     */
    float y;

    /** The rectangle of the line.
     *
     * The attributes of the rectangle are:
     *  - left: The rectangle.left() of the first character on the line.
     *  - right: The rectangle.right() of the last visible character on the line.
     *  - top: At the ascender of the line.
     *  - bottom: At the descender of the line.
     */
    aarectangle rectangle;

    /** The width of this line, excluding trailing white space, glyph morphing and kerning.
     */
    float width;

    /** Category of the last character on the line.
     * 
     * Use to determine if this line ends in:
     *  - Zp: An explicit paragraph separator.
     *  - Zl: An explicit line separator.
     *  - *: A word-wrapped line. Need to add line-separators into the stream for bidi-algorithm.
     */
    unicode_general_category last_category;

    /** The writing direction of the paragraph.
     *
     * This value will be set the same on each line of a paragraph.
     */
    unicode_bidi_class paragraph_direction;

    /** Construct a line.
     *
     * @param line_nr The line number counting from top to bottom.
     * @param begin The first character of the text.
     * @param first The first character of the line.
     * @param last One beyond the last character of the line.
     * @param width The width of the line.
     * @param metrics The initial line metrics.
     */
    text_shaper_line(
        size_t line_nr,
        const_iterator begin,
        iterator first,
        iterator last,
        float width,
        hi::font_metrics const &metrics) noexcept;

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return columns.size();
    }

    [[nodiscard]] constexpr iterator front() const noexcept
    {
        return columns.front();
    }

    [[nodiscard]] constexpr iterator back() const noexcept
    {
        return columns.back();
    }

    iterator operator[](size_t index) const noexcept
    {
        hi_assert_bounds(index, columns);
        return columns[index];
    }

    void layout(horizontal_alignment alignment, float min_x, float max_x, float sub_pixel_width) noexcept;

    /** Get the character nearest to position.
    * 
    * @return An iterator to the character, and true if the position is after the character.
     */
    [[nodiscard]] std::pair<const_iterator,bool> get_nearest(point2 position) const noexcept;
};

}
