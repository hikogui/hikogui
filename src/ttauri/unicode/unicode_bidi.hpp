// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_bidi_class.hpp"
#include "unicode_description.hpp"

namespace tt::inline v1 {

struct unicode_bidi_context {
    enum class mode_type : uint8_t { LTR, RTL, auto_LTR, auto_RTL };

    mode_type direction_mode = mode_type::auto_LTR;
    bool enable_mirrored_brackets = true;
    bool enable_line_separator = true;
    bool move_lf_and_ps_to_end_of_line = false;
};

namespace detail {

struct unicode_bidi_char_info {
    /** Index from the first character in the original list.
     */
    std::size_t index;

    /** Description of the code-point.
     */
    unicode_description const *description;

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

    [[nodiscard]] unicode_bidi_char_info(std::size_t index, unicode_description const &description) noexcept :
        index(index),
        description(&description),
        code_point(description.code_point()),
        embedding_level(0),
        direction(description.bidi_class()),
        bidi_class(description.bidi_class())
    {
    }

    /** Constructor for testing to bypass normal initialization.
     * WARNING: DO NOT USE EXCEPT IN UNIT TESTS.
     */
    [[nodiscard]] unicode_bidi_char_info(std::size_t index, unicode_bidi_class bidi_class) noexcept :
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

template<typename OutputIt, typename SetCodePoint, typename SetTextDirection>
static void unicode_bidi_L4(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    OutputIt output_it,
    SetCodePoint set_code_point,
    SetTextDirection set_text_direction) noexcept
{
    for (auto it = first; it != last; ++it, ++output_it) {
        ttlet text_direction = it->embedding_level % 2 == 0 ? unicode_bidi_class::L : unicode_bidi_class::R;
        set_text_direction(*output_it, text_direction);
        if (it->direction == unicode_bidi_class::R && it->description->bidi_bracket_type() != unicode_bidi_bracket_type::n) {
            set_code_point(*output_it, it->description->bidi_mirrored_glyph());
        }
    }
}

[[nodiscard]] std::pair<unicode_bidi_char_info_iterator, std::vector<unicode_bidi_class>> unicode_bidi_P1(
    unicode_bidi_char_info_iterator first,
    unicode_bidi_char_info_iterator last,
    unicode_bidi_context const &context = {}) noexcept;

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
 * The bidirectional algorithm will work correctly with either a list of code points
 * or a list of first-code-point-of-graphemes.
 *
 * @tparam It A Bidirectional read-write iterator.
 * @tparam GetChar function of the form: `(auto &) -> char32_t`.
 * @tparam SetChar function of the form: `(auto &, char32_t) -> void`.
 * @tparam SetTextDirection function of the form: `(auto &, unicode_bidi_class) -> void`
 * @param first The first iterator
 * @param last The last iterator
 * @param get_description A function to get the unicode description of an item.
 * @param set_char A function to set the character in an item.
 * @param set_text_direction A function to set the text direction in an item.
 * @return Iterator pointing one beyond the last element, the writing direction for each paragraph.
 */
template<typename It, typename GetDescription, typename SetCodePoint, typename SetTextDirection>
std::pair<It, std::vector<unicode_bidi_class>> unicode_bidi(
    It first,
    It last,
    GetDescription get_description,
    SetCodePoint set_code_point,
    SetTextDirection set_text_direction,
    unicode_bidi_context const &context = {})
{
    auto proxy = detail::unicode_bidi_char_info_vector{};
    proxy.reserve(std::distance(first, last));

    std::size_t index = 0;
    for (auto it = first; it != last; ++it) {
        proxy.emplace_back(index++, get_description(*it));
    }

    auto [proxy_last, paragraph_directions] = detail::unicode_bidi_P1(begin(proxy), end(proxy), context);
    last = shuffle_by_index(first, last, begin(proxy), proxy_last, [](ttlet &item) {
        return item.index;
    });

    detail::unicode_bidi_L4(
        begin(proxy),
        proxy_last,
        first,
        std::forward<SetCodePoint>(set_code_point),
        std::forward<SetTextDirection>(set_text_direction));
    return {last, std::move(paragraph_directions)};
}

/** Removes control characters which will not survive the bidi-algorithm.
* 
* All RLE, LRE, RLO, LRO, PDF, and BN characters are removed.
* 
* @post Control characters between the first and last iterators are moved to the end.
* @param first The first character.
* @param last One beyond the last character.
* @param description_func A function returning a `unicode_description const&` of the character.
* @return The iterator one beyond the last character that is valid.
*/
template<typename It, typename EndIt, typename DescriptionFunc>
It unicode_bidi_control_filter(It first, EndIt last, DescriptionFunc const &description_func)
{
    return std::remove_if(first, last, [&](ttlet &item) {
        return is_control(description_func(item).bidi_class());
    });
}


} // namespace tt::inline v1
