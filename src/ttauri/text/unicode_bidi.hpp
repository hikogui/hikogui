

#pragma once

#include "unicode_bidi_class.hpp"
#include "unicode_description.hpp"

namespace tt {
namespace detail {

struct unicode_bidi_char_info {
    /** Index from the first character in the original list.
     */
    size_t index;

    /** The current code point.
     * The value may change during the execution of the bidi algorithm.
     */
    char32_t code_point;

    /** Current computed direction of the code-point.
     * The value may change during the execution of the bidi algorithm.
     */
    unicode_bidi_class direction;

    /** The embedding level.
     * The value may change during the execution of the bidi algorithm.
     */
    int8_t embedding_level;

    /** Description of the code-point.
     */
    unicode_description const *description;

    [[nodiscard]] unicode_bidi_char_info(size_t index, char32_t code_point) noexcept :
        index(index),
        code_point(code_point),
        embedding_level(0)
    {
        description = &unicode_description_find(code_point);
        direction = description->bidi_class();
    }
};

using unicode_bidi_char_info_vector = std::vector<unicode_bidi_char_info>;
using unicode_bidi_char_info_iterator = unicode_bidi_char_info_vector::iterator;

struct unicode_bidi_paragraph {
    using characters_type = std::vector<unicode_bidi_char_info>;

    characters_type characters;

    template<typename... Args>
    void emplace_character(Args &&... args) noexcept
    {
        characters.emplace_back(std::forward<Args>(args)...);
    }
};

struct unicode_bidi_context {
    using paragraphs_type = std::vector<unicode_bidi_paragraph>;

    paragraphs_type paragraphs;

    unicode_bidi_paragraph &add_paragraph() noexcept
    {
        paragraphs.emplace_back();
        return paragraphs.back();
    }
};


[[nodiscard]] unicode_bidi_char_info_iterator
unicode_bidi_P1(unicode_bidi_char_info_iterator first, unicode_bidi_char_info_iterator last) noexcept;

}

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
template<typename It, typename GetChar, typename SetChar>
void unicode_bidi(It first, It last, GetChar get_char, SetChar set_char)
{
    auto text = detail::unicode_bidi_char_info_vector{};
    text.reserve(std::distance(first, last));

    size_t index = 0;
    for (auto it = first; it != last; ++it) {
        text.emplace_back(index++, get_char(it));
    }

    detail::unicode_bidi_P1(std::begin(text), std::end(text));
    
}

}

