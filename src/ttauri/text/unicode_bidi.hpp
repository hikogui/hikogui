// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#pragma once

#include "unicode_bidi_class.hpp"
#include "unicode_description.hpp"

namespace tt {
inline namespace v1 {
namespace detail {

struct unicode_bidi_char_info {
    /** Index from the first character in the original list.
     */
    size_t index;

    /** The current code point.
     * The value may change during the execution of the bidi algorithm.
     */
    char32_t code_point;

    /** The embedding level.
     * The value may change during the execution of the bidi algorithm.
     */
    int8_t embedding_level;

    /** Current computed direction of the code-point.
     * The value may change during the execution of the bidi algorithm.
     */
    unicode_bidi_class direction;

    /** The original bidi class of the code-point.
     * The value will NOT change during the execution of the bidi algorithm.
     */
    unicode_bidi_class bidi_class;

    /** Description of the code-point.
     */
    unicode_description const *description;

    [[nodiscard]] unicode_bidi_char_info(size_t index, char32_t code_point) noexcept :
        index(index), code_point(code_point), embedding_level(0)
    {
        description = &unicode_description_find(code_point);
        bidi_class = description->bidi_class();
        direction = bidi_class;
    }

    /** Constructor for testing to bypass normal initialization.
     * WARNING: DO NOT USE EXCEPT IN UNIT TESTS.
     */
    [[nodiscard]] unicode_bidi_char_info(size_t index, unicode_bidi_class bidi_class) noexcept :
        index(index),
        code_point(U'\ufffd'),
        direction(bidi_class),
        bidi_class(bidi_class),
        embedding_level(0),
        description(nullptr)
    {
    }
};

using unicode_bidi_char_info_vector = std::vector<unicode_bidi_char_info>;
using unicode_bidi_char_info_iterator = unicode_bidi_char_info_vector::iterator;
using unicode_bidi_char_info_const_iterator = unicode_bidi_char_info_vector::const_iterator;

struct unicode_bidi_paragraph {
    using characters_type = std::vector<unicode_bidi_char_info>;

    characters_type characters;

    template<typename... Args>
    void emplace_character(Args &&...args) noexcept
    {
        characters.emplace_back(std::forward<Args>(args)...);
    }
};

template<typename OutputIt, typename SetCodePoint>
static void unicode_bidi_L4(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    OutputIt output_it,
    SetCodePoint set_code_point) noexcept
{
    for (auto it = first; it != last; ++it, ++output_it) {
        if (it->direction == unicode_bidi_class::R && it->description->bidi_bracket_type() != unicode_bidi_bracket_type::n) {
            set_code_point(*output_it, it->description->bidi_mirrored_glyph());
        }
    }
}

struct unicode_bidi_test_parameters {
    unicode_bidi_class force_paragraph_direction = unicode_bidi_class::unknown;
    bool enable_mirrored_brackets = true;
    bool enable_line_separator = true;
};

[[nodiscard]] unicode_bidi_char_info_iterator unicode_bidi_P1(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    unicode_bidi_test_parameters test_parameters = {}) noexcept;

} // namespace detail

/** Reorder a given range of characters based on the unicode_bidi algorithm.
 * This algorithm will:
 *  - Reorder the list of items
 *  - Change code points to a mirrored version.
 *  - Remove code points which controls the bidirectional algorithm
 *
 * It is likely that an application has the characters grouped as graphemes
 * and is accompanied with the original index and possible other information.
 * The `get_char` function returns the first code-point of a grapheme. The
 * `set_char` function is used when the code-point needs to be replaced with
 * a mirrored version.
 *
 * The bidrectional algorithm will work correctly with either a list of code points
 * or a list of first-code-point-of-graphemes.
 *
 * @tparam It A Bidirectional read-write iterator.
 * @tparam GetChar function of the form: `(auto &) -> char32_t`.
 * @tparam SetChar function of the form: `(auto &, char32_t) -> void`.
 * @param first The first iterator
 * @param last The last iterator
 * @param get_char A function to get the character from an item.
 * @param set_char A function to set the character in an item.
 */
template<typename It, typename GetCodePoint, typename SetCodePoint>
It unicode_bidi(
    It first,
    It last,
    GetCodePoint get_code_point,
    SetCodePoint set_code_point,
    detail::unicode_bidi_test_parameters test_parameters = {})
{
    auto proxy = detail::unicode_bidi_char_info_vector{};
    proxy.reserve(std::distance(first, last));

    size_t index = 0;
    for (auto it = first; it != last; ++it) {
        proxy.emplace_back(index++, get_code_point(*it));
    }

    auto proxy_last = detail::unicode_bidi_P1(begin(proxy), end(proxy), test_parameters);
    last = shuffle_by_index(first, last, begin(proxy), proxy_last, [](ttlet &item) {
        return item.index;
    });

    detail::unicode_bidi_L4(begin(proxy), proxy_last, first, set_code_point);
    return last;
}

}
} // namespace tt
