// Copyright Take Vos 2020, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_normalization.hpp"
#include "unicode_description.hpp"
#include "ucd_normalize.hpp"
#include "../utility/module.hpp"
#include <string>
#include <algorithm>

namespace hi::inline v1 {

static void _unicode_decompose(char32_t code_point, unicode_normalization_config config, std::u32string &r) noexcept
{
    hilet &description = unicode_description::find(code_point);

    auto is_line_separator = false;
    is_line_separator |= code_point == unicode_LF and config.LF_is_line_separator;
    is_line_separator |= code_point == unicode_VT and config.VT_is_line_separator;
    is_line_separator |= code_point == unicode_FF and config.FF_is_line_separator;
    is_line_separator |= code_point == unicode_CR and config.CR_is_line_separator;
    is_line_separator |= code_point == unicode_NEL and config.NEL_is_line_separator;
    is_line_separator |= code_point == unicode_LS and config.LS_is_line_separator;
    is_line_separator |= code_point == unicode_PS and config.PS_is_line_separator;

    auto is_paragraph_separator = false;
    is_paragraph_separator |= code_point == unicode_LF and config.LF_is_paragraph_separator;
    is_paragraph_separator |= code_point == unicode_VT and config.VT_is_paragraph_separator;
    is_paragraph_separator |= code_point == unicode_FF and config.FF_is_paragraph_separator;
    is_paragraph_separator |= code_point == unicode_CR and config.CR_is_paragraph_separator;
    is_paragraph_separator |= code_point == unicode_NEL and config.NEL_is_paragraph_separator;
    is_paragraph_separator |= code_point == unicode_LS and config.LS_is_paragraph_separator;
    is_paragraph_separator |= code_point == unicode_PS and config.PS_is_paragraph_separator;

    auto drop = false;
    drop |= code_point == unicode_LF and config.drop_LF;
    drop |= code_point == unicode_VT and config.drop_VT;
    drop |= code_point == unicode_FF and config.drop_FF;
    drop |= code_point == unicode_CR and config.drop_CR;
    drop |= code_point == unicode_NEL and config.drop_NEL;
    drop |= code_point == unicode_LS and config.drop_LS;
    drop |= code_point == unicode_PS and config.drop_PS;
    drop |= ((code_point >= U'\u0000' and code_point <= U'\u001f') or code_point == U'\u007f')  and config.drop_C0;
    drop |= code_point >= U'\u0080' and code_point <= U'\u009f' and config.drop_C1;

    hilet decomposition_info = ucd_get_decomposition_info(code_point);
    if (is_line_separator) {
        r += config.line_separator_character;
        if (config.line_separator_character == unicode_CR) {
            r += unicode_LF;
        }

    } else if (is_paragraph_separator) {
        r += config.paragraph_separator_character;
        if (config.paragraph_separator_character == unicode_CR) {
            r += unicode_LF;
        }

    } else if (drop) {
        // This must come after checking for new-line which are explicitly converted.

    } else if (decomposition_info.should_decompose(config.decomposition_mask)) {
        for (hilet c : decomposition_info.decompose()) {
            _unicode_decompose(c, config, r);
        }

    } else {
        hilet ccc = ucd_get_canonical_combining_class(code_point);
        r += code_point | (wide_cast<char32_t>(ccc) << 24);
    }
}

static void _unicode_decompose(std::u32string_view text, unicode_normalization_config config, std::u32string &r) noexcept
{
    for (hilet c : text) {
        _unicode_decompose(c, config, r);
    }
}

static void _unicode_compose(std::u32string &text) noexcept
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

                hilet composed_code_point =  unicode_description::find(first_code_point).compose(second_code_point);
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

static void _unicode_reorder(std::u32string &text) noexcept
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

static void _unicode_clean(std::u32string &text) noexcept
{
    // clean up the text by removing the upper bits.
    for (auto &codePoint : text) {
        codePoint &= 0x1f'ffff;
    }
}

[[nodiscard]] std::u32string unicode_decompose(std::u32string_view text, unicode_normalization_config config) noexcept
{
    auto r = std::u32string{};
    _unicode_decompose(text, config, r);
    _unicode_reorder(r);
    _unicode_clean(r);
    return r;
}

[[nodiscard]] std::u32string unicode_normalize(std::u32string_view text, unicode_normalization_config config) noexcept
{
    auto r = std::u32string{};
    _unicode_decompose(text, config, r);
    _unicode_reorder(r);
    _unicode_compose(r);
    _unicode_clean(r);
    return r;
}

} // namespace hi::inline v1
