// Copyright Take Vos 2020, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_normalization.hpp"
#include "unicode_description.hpp"
#include "unicode_decomposition_type.hpp"
#include "../assert.hpp"
#include "../utility.hpp"
#include <string>
#include <algorithm>

namespace hi::inline v1 {

static void unicode_decompose(char32_t code_point, unicode_normalization_mask mask, std::u32string &r) noexcept
{
    hilet &description = unicode_description::find(code_point);

    if (any(mask & unicode_normalization_mask::decompose_newline) and (
        code_point == U'\n' or // Line Feed, LF U+000A
        code_point == U'\v' or // Vertical Tab, VT U+000B
        code_point == U'\f' or // Form Feed, FF U+000C
        code_point == U'\r' or // Carriage Return, LF U+000D
        code_point == U'\x85' or // Next Line, LF U+0085
        code_point == unicode_LS or // Line Separator, U+2028
        code_point == unicode_PS) // Paragraph Separator, U+2029
    ) {
        // Canonical combining class will be zero, so we can ignore it here.
        hilet paragraph_type = mask & unicode_normalization_mask::decompose_newline;
        if (paragraph_type == unicode_normalization_mask::decompose_newline_to_LF) {
            r += U'\n';
        } else if (paragraph_type == unicode_normalization_mask::decompose_newline_to_CRLF) {
            r += U'\r';
            r += U'\n';
        } else if (paragraph_type == unicode_normalization_mask::decompose_newline_to_PS) {
            r += unicode_PS;
        } else if (paragraph_type == unicode_normalization_mask::decompose_newline_to_SP) {
            r += U' ';
        }

    } else if (any(mask & unicode_normalization_mask::decompose_control) and is_C(description)) {
        // Control characters are dropped. (no operation)
        // This must come after checking for new-line which themselves are control characters.

    } else if (any(mask & description.decomposition_type())) {
        for (hilet c : description.decompose()) {
            unicode_decompose(c, mask, r);
        }

    } else {
        hilet ccc = description.canonical_combining_class();
        r += code_point | (wide_cast<char32_t>(ccc) << 24);
    }
}

static void unicode_decompose(std::u32string_view text, unicode_normalization_mask mask, std::u32string &r) noexcept
{
    for (hilet c : text) {
        unicode_decompose(c, mask, r);
    }
}

/**
 * @return The combined code-point, or U+ffff if first+second do not compose together.
 */
[[nodiscard]] static char32_t
unicode_compose(char32_t first, char32_t second, unicode_normalization_mask composition_mask) noexcept
{
    if (any(composition_mask & unicode_normalization_mask::compose_CRLF) and first == U'\r' and second == U'\n') {
        return U'\n';

    } else {
        return unicode_description::find(first).compose(second);
    }
}

static void unicode_compose(unicode_normalization_mask composition_mask, std::u32string &text) noexcept
{
    if (text.size() <= 1) {
        return;
    }

    std::size_t i = 0;
    std::size_t j = 0;
    while (i < text.size()) {
        hilet code_unit = text[i++];
        hilet code_point = code_unit & 0x1f'ffff;
        hilet combining_class = code_unit >> 24;
        hilet first_is_starter = combining_class == 0;

        if (code_point == U'\uffff') {
            // code-unit was snuffed out by compositing, skip it.

        } else if (first_is_starter) {
            // Try composing.
            auto first_code_point = code_point;
            char32_t previous_combining_class = 0;
            for (std::size_t k = i; k < text.size(); k++) {
                hilet second_code_unit = text[k];
                hilet second_code_point = second_code_unit & 0x1f'ffff;
                hilet second_combining_class = second_code_unit >> 24;

                bool blocking_pair = previous_combining_class != 0 && previous_combining_class >= second_combining_class;
                bool second_is_starter = second_combining_class == 0;

                hilet composed_code_point = unicode_compose(first_code_point, second_code_point, composition_mask);
                if (composed_code_point != U'\uffff' && !blocking_pair) {
                    // Found a composition.
                    first_code_point = composed_code_point;
                    // The canonical combined DecompositionOrder is always zero.
                    previous_combining_class = 0;
                    // Snuff out the code-unit.
                    text[k] = U'\uffff';

                } else if (second_is_starter) {
                    // End after failing to compose with the next start-character.
                    break;

                } else {
                    // The start character is not composing with this composingC.
                    previous_combining_class = second_combining_class;
                }
            }
            // Add the new combined character to the text.
            text[j++] = first_code_point;

        } else {
            // Unable to compose this character.
            text[j++] = code_point;
        }
    }

    text.resize(j);
}

static void unicode_reorder(std::u32string &text) noexcept
{
    for_each_cluster(
        text.begin(),
        text.end(),
        [](auto x) {
            return (x >> 21) == 0;
        },
        [](auto s, auto e) {
            std::stable_sort(s, e, [](auto a, auto b) {
                return (a >> 21) < (b >> 21);
            });
        });
}

static void unicode_clean(std::u32string &text) noexcept
{
    // clean up the text by removing the upper bits.
    for (auto &codePoint : text) {
        codePoint &= 0x1f'ffff;
    }
}

std::u32string unicode_NFD(std::u32string_view text, unicode_normalization_mask normalization_mask) noexcept
{
    auto r = std::u32string{};
    unicode_decompose(text, normalization_mask, r);
    unicode_reorder(r);
    unicode_clean(r);
    return r;
}

[[nodiscard]] std::u32string unicode_NFC(std::u32string_view text, unicode_normalization_mask normalization_mask) noexcept
{
    auto r = std::u32string{};
    unicode_decompose(text, normalization_mask, r);
    unicode_reorder(r);
    unicode_compose(normalization_mask, r);
    unicode_clean(r);
    return r;
}

std::u32string unicode_NFKD(std::u32string_view text, unicode_normalization_mask normalization_mask) noexcept
{
    auto r = std::u32string{};
    unicode_decompose(text, normalization_mask, r);
    unicode_reorder(r);
    unicode_clean(r);
    return r;
}

std::u32string unicode_NFKC(std::u32string_view text, unicode_normalization_mask normalization_mask) noexcept
{
    auto r = std::u32string{};
    unicode_decompose(text, normalization_mask, r);
    unicode_reorder(r);
    unicode_compose(normalization_mask, r);
    unicode_clean(r);
    return r;
}

} // namespace hi::inline v1
