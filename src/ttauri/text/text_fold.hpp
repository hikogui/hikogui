
#pragma once

#include "unicode_general_category.hpp"
#include "font_metrics.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../alignment.hpp"
#include "../coroutine.hpp"
#include <cstddef>

namespace tt::inline v1 {

/** Fold text to a given width.
 *
 * Folding happens at the start of each word. Words themselves are never folded and therefor the resulting lines
 * may be larger than the max_line_width.
 *
 * @param first Iterator of the first character of a text.
 * @param last Iterator to one beyond the last character of a text.
 * @param char_info_func A function taking an dereference of the iterator, returning a
 *                       pair of unicode_generic_category and its width.
 * @param max_line_width The maximum width of a line.
 * @return The number of characters in each line.
 */
template<typename It, typename ItEnd, typename CharInfoFunc>
[[nodiscard]] inline generator<std::size_t>
text_fold(It first, ItEnd last, float max_line_width, CharInfoFunc const &char_info_func) noexcept
{
    auto word_begin = 0_uz;
    auto word_width = 0.0f;

    auto line_begin = 0_uz;
    auto line_width = 0.0f;

    auto index = 0_uz;
    for (auto it = first; it != last; ++it, ++index) {
        ttlet[category, char_width] = char_info_func(*it);

        if (category == unicode_general_category::Zp or category == unicode_general_category::Zl) {
            // Found a line or paragraph separator; add all the characters including the separator.
            tt_axiom(index >= line_begin);
            co_yield index - line_begin + 1;

            line_width = 0.0f;
            line_begin = index + 1;
            word_width = 0.0f;
            word_begin = index + 1;

        } else if (category == unicode_general_category::Zs) {
            // Found a space; add all the characters including the space.

            // Extent the line with the word upto and including the space.
            // The new word starts at the space.
            // Add the length of the space to the new word.
            line_width += word_width + char_width;
            word_width = 0.0f;
            word_begin = index + 1;

        } else if (line_width == 0.0f and word_width + char_width > max_line_width) {
            // The word by itself on the line is too large. Just continue and wait for a white-space.
            word_width += char_width;

        } else if (line_width + word_width + char_width > max_line_width) {
            // Adding another character to the line makes it too long.
            // Break the line at the begin of the word.
            tt_axiom(word_begin > line_begin);
            co_yield word_begin - line_begin;

            // Start at a new line.
            line_width = 0.0f;
            line_begin = word_begin;
            word_width += char_width;

        } else {
            // Add the new character to the word.
            word_width += char_width;
        }
    }

    // The last line.
    tt_axiom(index >= line_begin);
    if (index > line_begin) {
        co_yield index - line_begin;
    }
}



/**
 */
template<typename It, typename ItEnd, typename CharInfoFunc, typename FontMetricFunc>
[[nodiscard]] inline generator<line_metrics> text_position_lines(
    It first,
    ItEnd last,
    float max_line_width,
    CharInfoFunc const &char_info_func,
    float line_spacing,
    float paragraph_spacing,
    FontMetricFunc const &font_metric_func) noexcept
{
    auto char_it = first;
    std::optional<line_metrics> prev_line = std::nullopt;
    for (ttlet num_characters : text_fold(first, last, max_line_width, char_info_func)) {
        auto line = line_metrics{};
        line.num_characters = num_characters;

        auto non_added_width = 0.0f;
        for (auto i = 0_uz; i != num_characters; ++i) {
            ttlet &c = *it++;
            ttlet[category, glyph_width] = char_info_func(c);
            ttlet font_metrics = font_metric_func(c);

            line.metrics = max(line.metrics, font_metrics);
            line.ends_paragraph |= category == unicode_general_category::Zp;

            // Only add the width of a space if it is followed by a non-space.
            non_added_width += glyph_width;
            if (not is_Z(category)) {
                line.width += non_added_width;
                non_added_width = 0.0f;
            }
        }

        if (prev_line) {
            // Calculate the natural distance between lines based on the font metrics.
            ttlet natural_line_distance = prev_line->metrics.descender + line.metrics.ascender +
                std::max(prev_line->metrics.line_gap, line.metrics.line_gap);

            // Adjust the line distance on the paragraph- and line-spacing multipliers.
            ttlet line_distance = natural_line_distance * prev_line->ends_paragraph ? paragraph_spacing : line_spacing;

            // Round line position to full pixels.
            line.y = std::round(prev_line->y - line_distance);
        }

        co_yield line;

        prev_line = line;
    }
}

/** Calculate the bounding box around line metrics.
 *
 * @param first A iterator to the first line metrics element.
 * @param last A iterator one beyond the last line metrics element
 */
template<typename It, typename ItEnd>
[[nodiscard]] inline aarectangle text_bounding_rectangle(It first, ItEnd last)
{
    if (first == last) {
        return {};
    }

    ttlet y_top = first->y + first->metrics.x_height;
    ttlet y_bottom = last->y;
    auto width = 0.0f;
    for (auto it = first; it != last; ++it) {
        inplace_max(width, it->width);
    }
    return {point2{0.0f, y_bottom}, point2{width, y_top}};
}

/** Vertically align the given line metrics.
* 
* @param first A iterator to the first line metrics element.
* @param last A iterator one beyond the last line metrics element
* @param alignment How to vertically align the lines.
*/
template<typename It, typename ItEnd>
[[nodiscard]] inline void text_vertical_align(It first, ItEnd last, vertical_alignment alignment)
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

} // namespace tt::inline v1
