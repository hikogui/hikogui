// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_grapheme_cluster_break.hpp"
#include "unicode_description.hpp"

namespace hi::inline v1 {

struct grapheme_break_state {
    unicode_grapheme_cluster_break previous = unicode_grapheme_cluster_break::Other;
    int RI_count = 0;
    bool first_character = true;
    bool in_extended_pictograph = false;

    void reset() noexcept
    {
        previous = unicode_grapheme_cluster_break::Other;
        RI_count = 0;
        first_character = true;
        in_extended_pictograph = false;
    }
};

/** Check if for a grapheme break before the given code-point.
 * Code points must be tested in order, starting at the beginning of the text.
 *
 * @param code_point Current code point to test.
 * @param state Current state of the grapheme-break algorithm.
 * @return true when a grapheme break exists before the current code-point.
 */
[[nodiscard]] bool breaks_grapheme(char32_t code_point, grapheme_break_state& state) noexcept;

template<typename It, typename ItEnd>
[[nodiscard]] std::vector<unicode_break_opportunity> unicode_grapheme_break(It first, ItEnd last) noexcept
{
    auto r = std::vector<unicode_break_opportunity>{};
    auto state = grapheme_break_state{};

    for (auto it = first; it != last; ++it) {
        hilet opertunity = breaks_grapheme(*it, state) ? unicode_break_opportunity::yes : unicode_break_opportunity::no;
        r.push_back(opertunity);
    }

    r.push_back(unicode_break_opportunity::yes);
    return r;
}

/** Wrap lines in text that are too wide.
 * This algorithm may modify white-space in text and change them into line seperators.
 * Lines are separated using the U+2028 code-point, and paragraphs are separated by
 * the U+2029 code-point.
 *
 * @param first The first iterator of a text to wrap
 * @param last The one beyond the last iterator of a text to wrap
 * @param max_width The maximum width of a line.
 * @param get_width A function returning the width of an item pointed by the iterator.
 *                  `float get_width(auto const &item)`
 * @param get_code_point A function returning the code-point of an item pointed by the iterator.
 *                       `char32_t get_code_point(auto const &item)`
 * @param set_code_point A function changing the code-point of an item pointed by the iterator.
 *                       `void set_code_point(auto &item, char32_t code_point)`
 */
void wrap_lines(auto first, auto last, float max_width, auto get_width, auto get_code_point, auto set_code_point) noexcept
{
    using enum unicode_general_category;

    auto it_at_last_space = last;
    float width_at_last_space = 0.0;
    float current_width = 0.0;

    for (auto it = first; it != last; ++it) {
        hilet code_point = get_code_point(*it);
        hilet description = unicode_description::find(code_point);
        hilet general_category = description->general_category();

        if (general_category == Zp || general_category == Zl) {
            // Reset the line on existing line and paragraph separator.
            it_at_last_space = last;
            width_at_last_space = 0.0f;
            current_width = 0.0;
            continue;

        } else if (general_category == Zs) {
            // Remember the length of the line at the end of the word.
            it_at_last_space = it;
            width_at_last_space = current_width;
        }

        current_width += get_width(*it);
        if (current_width >= max_width && it_at_last_space != last) {
            // The line is too long, replace the last space with a line separator.
            set_code_point(*it, U'\u2028');
            it_at_last_space = last;
            width_at_last_space = 0.0f;
            current_width = 0.0;
            continue;
        }
    }
}

} // namespace hi::inline v1
