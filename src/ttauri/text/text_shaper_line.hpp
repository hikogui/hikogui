// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_shaper_char.hpp"
#include "font_metrics.hpp"
#include "../unicode/unicode_bidi_class.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../alignment.hpp"
#include <vector>

namespace tt::inline v1 {

class text_shaper_line {
public:
    using iterator = std::vector<text_shaper_char>::iterator;
    using const_iterator = std::vector<text_shaper_char>::const_iterator;
    using column_vector = std::vector<iterator>;

    const_iterator first;
    const_iterator last;

    /** Indices to the characters in the text.
     *
     * The indices are in display-order.
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

    /** True if this line ends a paragraph.
     */
    bool end_of_paragraph;

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
     */
    text_shaper_line(size_t line_nr, const_iterator begin, const_iterator first, const_iterator last, float width) noexcept;

    void layout(horizontal_alignment alignment, float min_x, float max_x, float sub_pixel_width) noexcept;

    /** Get the character nearest to position.
    * 
    * @return An iterator to the character, and true if the position is after the character.
     */
    [[nodiscard]] std::pair<const_iterator,bool> get_nearest(point2 position) const noexcept;
};

}
