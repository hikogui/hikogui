

#pragma once

#include "unicode_data.hpp"
#include "unicode_bidi_class.hpp"

namespace tt {

struct unicode_bidi_char_info {
    /** Index from the first character in the original list.
     */
    size_t index;

    /** Code point.
     */
    char32_t code_point;

    /** Current bidirectional class of the code-point.
     */
    unicode_bidi_class bidi_class;

    /** Original bidirectional class of the code-point used by L1.
     */
    unicode_bidi_class original_bidi_class;

    /** The embedding level.
     */
    int8_t embedding_level;

    [[nodiscard]] unicode_bidi_char_info(size_t index, char32_t code_point) noexcept :
        index(index),
        code_point(code_point),
        bidi_class(unicode_data::global->get_bidi_class(code_point)),
        original_bidi_class(bidi_class),
        embedding_level(0) {}
};

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

void unicode_bidi_P1(unicode_bidi_paragraph &paragraph);

template<typename It, typename GetChar, typename SetChar>
void unicode_bidi_P1(unicode_bidi_context &context, It first, It last, GetChar get_char)
{
    size_t index = 0;
    context.add_paragraph()

    auto paragraph = unicode_bid_paragraph{};
    for (auto it = first; it != last; ++it) {
        auto code_point = get_char(*it);
        auto bidi_class = unicode_data::global->get_bidi_class(code_point)

        paragraph->emplace_character(index++, code_point, bidi_class);

        if (bidi_class == unicode_bidi_class::B) {
            unicode_bidi_P1(paragraph);
        }
    }

    unicode_bidi_P1(paragraph);
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
    auto context = unicode_bidi_context{first, last, get_char};

    unicode_bidi_P1_P3(context);
    unicode_bidi_X1_X8(context);
    unicode_bidi_X10(context);
    unicode_bidi_W(context);
    unicode_bidi_N(context);
    unicode_bidi_I1_I2(context);
    unicode_bidi_L(context);
}

}

