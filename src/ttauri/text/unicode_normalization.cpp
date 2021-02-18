// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_normalization.hpp"
#include "unicode_description.hpp"
#include "unicode_composition.hpp"
#include "ttauri/text/unicode_db.hpp"
#include "../assert.hpp"
#include "../required.hpp"

namespace tt {

/** Detect typographical ligature.
 * A typographical ligatures will have the same meaning in the text
 * when it is in composed or decomposed form.
 */
static bool is_typographical_ligature(char32_t codePoint)
{
    switch (codePoint) {
    case 0xfb00: // ff
    case 0xfb01: // fi
    case 0xfb02: // fl
    case 0xfb03: // ffi
    case 0xfb04: // ffl
    case 0xfb05: // long st
    case 0xfb06: // st
    case 0xfb13: // men now
    case 0xfb14: // men ech
    case 0xfb15: // men ini
    case 0xfb16: // vew now
    case 0xfb17: // men xeh
        return true;
    default: return false;
    }
}

static void unicode_decompose(char32_t code_point, bool compatible, bool ligature, bool paragraph, std::u32string &r) noexcept
{
    ttlet &description = unicode_description_find(code_point);

    ttlet must_decompose =
        (compatible || description.decomposition_canonical() || (ligature && is_typographical_ligature(code_point)));

    if (paragraph && code_point == U'\n') {
        r += U'\u2029'; // paragraph separator.

    } else if (is_hangul_syllable(code_point)) {
        ttlet S_index = code_point - detail::unicode_hangul_S_base;
        ttlet L_index = static_cast<char32_t>(S_index / detail::unicode_hangul_N_count);
        ttlet V_index = static_cast<char32_t>((S_index % detail::unicode_hangul_N_count) / detail::unicode_hangul_T_count);
        ttlet T_index = static_cast<char32_t>(S_index % detail::unicode_hangul_T_count);

        unicode_decompose(detail::unicode_hangul_L_base + L_index, compatible, ligature, paragraph, r);
        unicode_decompose(detail::unicode_hangul_V_base + V_index, compatible, ligature, paragraph, r);

        if (T_index > 0) {
            unicode_decompose(detail::unicode_hangul_T_base + T_index, compatible, ligature, paragraph, r);
        }

    } else if (must_decompose) {
        if (description.decomposition_length() == 0) {
            r += code_point | (static_cast<char32_t>(description.combining_class()) << 24);

        } else if (description.decomposition_length() == 1) {
            unicode_decompose(static_cast<char32_t>(description.decomposition_index()), compatible, ligature, paragraph, r);

        } else if (description.composition_canonical() && description.decomposition_length() == 2) {
            tt_axiom(description.decomposition_index() < std::size(detail::unicode_db_composition_table));
            ttlet &composition = detail::unicode_db_composition_table[description.decomposition_index()];

            unicode_decompose(composition.first(), compatible, ligature, paragraph, r);
            unicode_decompose(composition.second(), compatible, ligature, paragraph, r);

        } else {
            tt_axiom(
                description.decomposition_index() + description.decomposition_length() <=
                std::size(detail::unicode_db_decomposition_table));

            auto it = std::begin(detail::unicode_db_decomposition_table) + description.decomposition_index();

            for (size_t i = 0; i != description.decomposition_length(); ++i) {
                unicode_decompose(*(it++), compatible, ligature, paragraph, r);
            }
        }

    } else {
        r += code_point | (static_cast<char32_t>(description.combining_class()) << 24);
    }
}

static void
unicode_decompose(std::u32string_view text, bool compatible, bool ligature, bool paragraph, std::u32string &r) noexcept
{
    for (ttlet c: text) {
        unicode_decompose(c, compatible, ligature, paragraph, r);
    }
}

[[nodiscard]] static char32_t unicode_compose(char32_t first, char32_t second, bool paragraph, bool composeCRLF) noexcept
{
    if (composeCRLF && first == U'\r' && second == U'\n') {
        return paragraph ? U'\u2029' : U'\n';

    } else if (composeCRLF && first == U'\r' && second == U'\u2029') {
        return U'\u2029';

    } else if (is_hangul_L_part(first) && is_hangul_V_part(second)) {
        ttlet L_index = first - detail::unicode_hangul_L_base;
        ttlet V_index = second - detail::unicode_hangul_V_base;
        ttlet LV_index = L_index * detail::unicode_hangul_N_count + V_index * detail::unicode_hangul_T_count;
        return detail::unicode_hangul_S_base + LV_index;

    } else if (is_hangul_LV_part(first) && is_hangul_T_part(second)) {
        ttlet T_index = second - detail::unicode_hangul_T_base;
        return first + T_index;

    } else {
        return unicode_composition_find(first, second);
    }
}

static void unicode_compose(bool paragraph, bool composeCRLF, std::u32string &text) noexcept
{
    if (text.size() <= 1) {
        return;
    }

    size_t i = 0;
    size_t j = 0;
    while (i < text.size()) {
        ttlet code_unit = text[i++];
        ttlet code_point = code_unit & 0x1f'ffff;
        ttlet combining_class = code_unit >> 24;
        ttlet first_is_starter = combining_class == 0;

        if (code_point == U'\uffff') {
            // code-unit was snuffed out by compositing, skip it.

        } else if (first_is_starter) {
            // Try composing.
            auto first_code_point = code_point;
            char32_t previous_combining_class = 0;
            for (size_t k = i; k < text.size(); k++) {
                ttlet second_code_unit = text[k];
                ttlet second_code_point = second_code_unit & 0x1f'ffff;
                ttlet second_combining_class = second_code_unit >> 24;

                bool blocking_pair = previous_combining_class != 0 && previous_combining_class >= second_combining_class;
                bool second_is_starter = second_combining_class == 0;

                ttlet composed_code_point = unicode_compose(first_code_point, second_code_point, paragraph, composeCRLF);
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

std::u32string unicode_NFD(std::u32string_view text, bool ligatures, bool paragraph) noexcept
{
    auto r = std::u32string{};
    unicode_decompose(text, false, ligatures, paragraph, r);
    unicode_reorder(r);
    unicode_clean(r);
    return r;
}

[[nodiscard]] std::u32string
unicode_NFC(std::u32string_view text, bool ligatures, bool paragraph, bool composeCRLF) noexcept
{
    auto r = std::u32string{};
    unicode_decompose(text, false, ligatures, paragraph, r);
    unicode_reorder(r);
    unicode_compose(paragraph, composeCRLF, r);
    unicode_clean(r);
    return r;
}

std::u32string unicode_NFKD(std::u32string_view text, bool paragraph) noexcept
{
    auto r = std::u32string{};
    unicode_decompose(text, true, false, paragraph, r);
    unicode_reorder(r);
    unicode_clean(r);
    return r;
}

std::u32string unicode_NFKC(std::u32string_view text, bool paragraph, bool composeCRLF) noexcept
{
    auto r = std::u32string{};
    unicode_decompose(text, true, false, paragraph, r);
    unicode_reorder(r);
    unicode_compose(paragraph, composeCRLF, r);
    unicode_clean(r);
    return r;
}

}
