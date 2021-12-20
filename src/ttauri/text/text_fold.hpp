
#pragma once

#include "unicode_general_category.hpp"
#include "../coroutine.hpp"
#include <cstddef>

namespace tt::inline v1 {

/**
 * @param first Iterator of the first character of a text.
 * @param last Iterator to one beyond the last character of a text.
 * @param char_info_func A function taking an derefence of the iterator, returing a
 *                       pair of unicode_generic_category and its width.
 * @param max_line_width The maximum width of a line.
 * @return A list of line sizes.
 */
template<typename It, typename ItEnd, typename CharInfoFunc>
[[nodiscard]] constexpr generator<std::size_t> text_fold(
    It first,
    ItEnd last,
    float max_line_width,
    CharInfoFunc const &char_info_func) const noexcept
{
    auto word_begin = 0_uz;
    auto word_width = 0.0f;

    auto line_begin = 0_uz;
    auto line_width = 0.0f;

    auto index = 0_uz;
    for (auto it = first; it != last; ++it, ++index) {
        ttlet [category, char_width] = char_info_func(*it);

        if (category == unicode_general_category::Zp or category == unicode_general_category::Zl) {
            // Found a line or paragraph separator; add all the characters including the separator.
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

        } else if (line_width == 0.0f and word_width + char_width > width) {
            // The word by itself on the line is too large. Just continue and wait for a white-space.
            word_width += char_width;

        } else if (line_width + word_width + char_width > width) {
            // Adding another character to the line makes it too long.
            // Break the line at the begin of the word.
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
    co_yield index - line_begin;
}

}
