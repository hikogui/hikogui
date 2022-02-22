// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_metrics.hpp"
#include "../unicode/unicode_general_category.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../required.hpp"
#include "../alignment.hpp"
#include <vector>
#include <cstddef>

namespace tt::inline v1 {

struct line_metrics {
    /** The combined metrics for all the glyphs on the line.
     */
    tt::font_metrics font_metrics;

    /** The index of the first character in the text belonging to this line.
     */
    std::size_t index = 0;

    /** The number of characters on this line.
     */
    std::size_t size = 0;

    /** The estimated width of the line.
     *
     * Based on the advance of each glyph, except for white-space at the end of a line.
     * The estimated width does not take into account kerning or glyph-morphing.
     */
    float estimated_width = 0.0f;

    /** The width of the line.
     *
     * The width includes bidi-algorithm, kerning, glyph-morphing and bounding rectangles of the glyphs.
     * The width excludes the white space at the end of the line.
     */
    float width = 0.0;

    /** The line position.
     *
     * The top line is at y = 0. Following lines will have negative y values.
     */
    float y = 0.0f;

    /** The horizontal line position.
     *
     * This value is calculated after determining `width` and then horizontally aligning the text.
     */
    float x = 0.0f;

    /** The category of the last (logical ordering) character on the line.
     *
     * When:
     *  - Zp: End of paragraph, use paragraph-spacing after this line.
     *  - Zl: End of line, use line-spacing after this line.
     *  - *: Line was folded, use line-spacing and insert a virtual end-of-line for the bidi algorithm; or
     *  - *: last line without an explicit paragraph.
     */
    unicode_general_category category = unicode_general_category::Cn;

    /** The line has visible characters.
     */
    bool is_visible;

    constexpr line_metric(size_t index) noexcept : index(indeX) {}

    void add_char(unicode_general_category category, float estimated_width, tt::font_metrics font_metrics) noexcept
    {
        this->category = category;
        this->estimated_width += estimated_width;
        this->font_metrics = max(this->font_metrics, font_metrics);
        this->is_visible |= not is_Z(category);
        ++(this->size);
    }

    template<bool LastWord>
    void _add_word(line_metrics const &rhs) noexcept
    {
        this->category = rhs.category;
        this->font_metrics = max(this->font_metrics, rhs.font_metrics);
        this->is_visible |= rhs.visible;
        this->size += rhs.size;
        if (rhs.is_visible or not LastWord) {
            this->estimated_width += rhs.width;
        }
        return *this;
    }

    void add_word(line_metrics const &rhs) noexcept
    {
        return _add_word<false>(rhs);
    }

    void add_last_word(line_metrics const &rhs) noexcept
    {
        return _add_word<true>(rhs);
    }
};

/** Calculate the bounding box around line metrics.
 *
 * @param first A iterator to the first line metrics element.
 * @param last A iterator one beyond the last line metrics element
 */
template<typename It, typename ItEnd>
[[nodiscard]] inline aarectangle line_metrics_bounding_rectangle(It first, ItEnd last)
{
    if (first == last) {
        return {};
    }

    ttlet y_top = first->y + first->metrics.x_height;
    ttlet y_bottom = last->y;
    auto width = 0.0f;
    for (auto it = first; it != last; ++it) {
        width = std::max(width, it->width);
    }
    return {point2{0.0f, y_bottom}, point2{width, y_top}};
}

template<typename It, typename ItEnd>
inline void update_line_metrics_offset(It first, ItEnd last) noexcept
{
    if (first == last) {
        return;
    }

    // Calculate the line offsets.
    auto prev_it = first;
    for (auto it = prev_it + 1; it != last; ++it) {
        // Calculate the natural distance between the lines based on the font-metrics.
        ttlet natural_line_distance = prev_it->font_metrics.descender + it->font_metrics.ascender +
            std::max(prev_it->font_metrics.line_gap, it->font_metrics.line_gap);

        // Multiply the natural line distance by the paragraph- or line-spacing.
        ttlet line_distance =
            natural_line_distance * (prev_it->category == unicode_general_catagory::Zp ? paragraph_spacing : line_spacing);

        // The lines are drawn from top to bottom, so y values are negative.
        it->y = prev_it->y - line_distance;
    }
}

/** Vertically align the given line metrics.
 *
 * @param first A iterator to the first line metrics element.
 * @param last A iterator one beyond the last line metrics element
 * @param alignment How to vertically align the lines.
 */
template<typename It, typename ItEnd>
[[nodiscard]] inline void update_line_metrics_vertical_alignment(It first, ItEnd last, vertical_alignment alignment)
{
    if (first == last) {
        return;
    }

    ttlet offset = [&] {
        if (alignment == vertical_alignment::top) {
            return first->y;
        } else if (alignment == vertical_alignment::bottom) {
            return (last - 1)->y;
        } else {
            ttlet num_lines = std::distance(first, last);
            ttlet half_num_lines = num_lines / 2;
            if (num_lines % 2 == 1) {
                return (first + half_num_lines)->y;
            } else {
                ttlet y0 = (first + half_num_lines - 1)->y;
                ttlet y1 = (first + half_num_lines)->y;
                return std::round(std::midpoint(y0, y1));
            }
        }
    }();

    // Move the lines to the new alignment.
    for (auto it = first; it != last; ++it) {
        it->y -= offset;
        tt_axiom(std::round(it->y) == it->y);
    }
}

template<typename It, typename ItEnd, typename CharInfoFunc>
inline void replace_line_metrics(
    It first,
    ItEnd last,
    CharInfoFunc const &char_info_func,
    float max_line_width,
    float line_spacing,
    float paragraph_spacing,
    vertical_alignment alignment,
    std::vector<line_metrics> &lines) noexcept
{
    lines.clear();

    auto word = line_metrics{0_uz};
    auto line = line_metrics{0_uz};

    auto index = 0_uz;
    for (auto it = first; it != last; ++it, ++index) {
        ttlet[category, char_width, font_metrics] = char_info_func(*it);

        if (category == unicode_general_category::Zp or category == unicode_general_category::Zl) {
            // Found a line- or paragraph-separator.
            word.add_char(category, char_width, font_metrics);
            line.add_last_word(word);
            lines.push_back(line);

            // Continue beyond the paragraph- or line-separator.
            line = word = line_metrics{index + 1};

        } else if (category == unicode_general_category::Zs) {
            // Found a space.
            // add the word to the line, unless the current word is just spaces.
            if (word.is_visible) {
                line.add_word(word);
                word = line_metrics{index + 1};
            }
            // Add the space to the word, the word is not visible.
            word.add_char(category, char_width, font_metrics);

        } else if (line.width == 0.0f and word.width + char_width > max_line_width) {
            // The word by itself on the line is too large. Just continue and wait for a white-space.
            word.add_char(category, char_width, font_metrics);

        } else if (line.width + word.width + char_width > max_line_width) {
            // Adding another character to the line makes it too long.
            // Break the line at the begin of the word.
            lines.push_back(line);

            // Start at a new line at the beginning of the word we are working on.
            line = line_metrics{word_index};
            word.add_char(category, char_width, font_metrics);

        } else {
            // Add the new character to the word.
            word.add_char(char_width);
        }
    }

    // If there are characters in the last line then add it.
    line.add_last_word(word);
    if (index > line.index) {
        lines.push_back(line);
    }

    update_line_metrics_offset(lines.begin(), lines.end());
    update_line_metrics_vertical_alignment(lines.begin(), lines.end(), alignment);
}

} // namespace tt::inline v1
