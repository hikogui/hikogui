// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode/unicode_plural.hpp Functions implementing the unicode-plural rules.
 *
 * See: https://www.unicode.org/cldr/cldr-aux/charts/35/supplemental/language_plural_rules.html
 */

module;
#include "../macros.hpp"

#include <concepts>
#include <array>
#include <cstdint>
#include <cstddef>
#include <bit>

export module hikogui_unicode_unicode_plural;
import hikogui_coroutine_generator; // XXX #616
import hikogui_i18n;
import hikogui_utility;


export namespace hi { inline namespace v1 {

/** The plurality value of a cardinal or ordinal number.
 */
enum class plurality_value : uint8_t {
    /** The number was zero, and this means something in the current language.
     */
    zero = 0,

    /** The number was one, and this means something in the current language.
     */
    one = 1,

    /** The number was two, and this means something in the current language.
     */
    two = 2,

    /** The number is part of few, and this means something in the current language.
     */
    few = 3,

    /** The number is part of many, and this means something in the current language.
     */
    many = 4,

    /** Any other number, every language will have at least this.
     */
    other = 5
};

/** A mask of plurality values that this language supports.
 */
enum class plurality_mask : uint8_t {
    /** The number was zero, and this means something in the current language.
     */
    zero = 1 << std::to_underlying(plurality_value::zero),

    /** The number was one, and this means something in the current language.
     */
    one = 1 << std::to_underlying(plurality_value::one),

    /** The number was two, and this means something in the current language.
     */
    two = 1 << std::to_underlying(plurality_value::two),

    /** The number is part of few, and this means something in the current language.
     */
    few = 1 << std::to_underlying(plurality_value::few),

    /** The number is part of many, and this means something in the current language.
     */
    many = 1 << std::to_underlying(plurality_value::many),

    /** Any other number, every language will have at least this.
     */
    other = 1 << std::to_underlying(plurality_value::other),
};

/** Or plurality masks together.
 */
[[nodiscard]] constexpr plurality_mask operator|(plurality_mask const& lhs, plurality_mask const& rhs) noexcept
{
    return static_cast<plurality_mask>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

/** Plurality of a number.
 */
struct plurality {
    plurality_value value;
    plurality_mask mask;

    constexpr plurality(plurality_value value, plurality_mask mask) noexcept : value(value), mask(mask)
    {
        // Check if the value uses only bits that are set in the mask.
        hi_axiom(not to_bool((1 << std::to_underlying(value)) & ~std::to_underlying(mask)));
    }

    /** Get an index to select between translations.
     *
     * @param n The number of plural messages for this translation.
     * @return The index into the plural message table for this translation.
     *         If there are not enough messages, then the index to the last message
     *         is returned.
     */
    [[nodiscard]] constexpr size_t index(size_t n) const noexcept
    {
        hi_assert(n != 0);

        hilet value_as_mask = (1 << (std::to_underlying(value) + 1)) - 1;
        // Get the index based on the number of '1' bits that are set from the
        // plurality position to lsb.
        hilet i = std::popcount(narrow_cast<uint8_t>(value_as_mask & std::to_underlying(mask))) - 1;
        if (i < n) {
            return i;
        } else {
            return n - 1;
        }
    }
};

namespace detail {

/** The operand for the unicode-plural rules.
 *
 * This operand extracts information from a number.
 */
struct plural_operand {
    /** Absolute value.
     */
    unsigned long long n = 0;

    /** Visible fraction digits with trailing zeros.
     */
    unsigned long long f = 0;

    /** Visible fraction digits without trailing zeros.
     */
    unsigned long long t = 0;

    /** Number of digits in the value.
     */
    uint8_t i = 0;

    /** Number of visible fraction digits with trailing zeros.
     */
    uint8_t v = 0;

    /** Number of visible fraction digits without trailing zeros.
     */
    uint8_t w = 0;

    /** Compact decimal exponent value.
     */
    uint8_t c = 0;

    constexpr plural_operand(std::integral auto value) noexcept : n(std::abs(value)), i(narrow_cast<uint8_t>(decimal_width(value))) {}
};

// Each of the rules can be shared by multiple languages. Each rule will be named
// after the first language in alphabetical order.

// clang-format off
[[nodiscard]] constexpr plurality cardinal_plural_bambara(plural_operand) noexcept
{
    return {plurality_value::other, plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_cebuano(plural_operand op) noexcept
{
    hilet value = [&] {
        if (
            (op.v == 0 and op.i >= 1 and op.i <= 3) or
            (op.v == 0 and op.i % 10 != 4 and op.i % 10 != 6 and op.i % 10 != 9) or
            (op.v != 0 and op.f % 10 != 4 and op.f % 10 != 6 and op.f % 10 != 9)) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_central_atlas_tamazight(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 0 or op.n == 1 or (op.n >= 11 and op.n <= 99)) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_icelandic(plural_operand op) noexcept
{
    hilet value = [&] {
        if ((op.t == 0 and op.i % 10 == 1 and op.i % 100 != 11) or op.t != 0) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_akan(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 0 or op.n == 1) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_afrikaans(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 1) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_latvian(plural_operand op) noexcept
{
    hilet value = [&] {
        if (
            op.n == 0 or
            (op.n % 100 >= 11 and op.n % 100 <= 19) or
            (op.v == 2 and op.f % 100 >= 11 and op.f % 100 <= 19)) {
            return plurality_value::zero;
        } else if (
            (op.n % 10 == 1 and op.n % 100 != 11) or
            (op.v == 2 and op.f % 10 == 1 and op.f % 100 != 11) or
            (op.v != 2 and op.f % 10 == 1)) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::zero | plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_colognian(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 0) {
            return plurality_value::zero;
        } else if (op.n == 1) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::zero | plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_inari_sami(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 1) {
            return plurality_value::one;
        } else if (op.n == 2) {
            return plurality_value::two;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::two | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_belarusian(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n % 10 == 1 and op.n % 100 != 11) {
            return plurality_value::one;
        } else if (op.n % 10 >= 2 and op.n % 10 <= 4 and not (op.n % 100 >= 12 and op.n % 100 <= 14)) {
            return plurality_value::few;
        } else if (
            op.n % 10 == 0 or
            (op.n % 10 >= 5 and op.n % 10 <= 9) or
            (op.n % 100 >= 11 and op.n % 100 <= 14)) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::few | plurality_mask::many | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_polish(plural_operand op) noexcept
{
    hilet value = [&] {
        // The specification uses op.i (number of digits) instead of op.n (absolute value)
        if (op.n == 1 and op.v == 0) {
            return plurality_value::one;
        } else if (op.v == 0 and (op.n % 10 >= 2 and op.n % 10 <= 4) and not (op.n % 100 >= 12 and op.n % 100 <= 14)) {
            return plurality_value::few;
        } else if (
            (op.v == 0 and op.n != 1 and op.n % 10 >= 0 and op.n % 10 <= 1) or
            (op.v == 0 and op.n % 10 >= 5 and op.n % 10 <= 9) or
            (op.v == 0 and op.n % 100 >= 12 and op.n % 100 <= 14)) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::few | plurality_mask::many | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_lithuanian(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n % 10 == 1 and not (op.n % 100 >= 11 and op.n % 100 <= 19)) {
            return plurality_value::one;
        } else if (op.n % 10 >= 2 and op.n % 10 <= 9 and not (op.n % 100 >= 11 and op.n % 100 <= 19)) {
            return plurality_value::few;
        } else if (op.f != 0) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::few | plurality_mask::many | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_bosnian(plural_operand op) noexcept
{
    hilet value = [&] {
        // The specification uses op.i (number of digits) instead of op.n (absolute value)
        if (
            (op.v == 0 and op.n % 10 == 1 and op.n % 100 != 11) or
            (op.f % 10 == 1 and op.f % 100 != 11)) {
            return plurality_value::one;
        } else if (
            (op.v == 0 and op.n % 10 >= 2 and op.n % 10 <= 4 and not (op.n % 100 >= 12 and op.n % 100 <= 14)) or
            (op.f % 10 >= 2 and op.f % 10 <= 4 and not (op.f % 100 >= 12 and op.n % 100 <= 14))) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::few | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_tachelhit(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.i == 0 or op.n == 1) {
            return plurality_value::one;
        } else if (op.n >= 2 and op.n <= 10) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::few | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_moldavian(plural_operand op) noexcept
{
    hilet value = [&] {
        // The specification uses op.i (number of digits) for determining 'one'
        // which is wrong.
        if (op.n == 1 and op.v == 0) {
            return plurality_value::one;
        } else if (
            op.v != 0 or
            op.n == 0 or
            (op.n % 100 >= 2 and op.n % 100 <= 19)) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::few | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_czech(plural_operand op) noexcept
{
    hilet value = [&] {
        // The specification uses op.i (number of digits) instead of
        // op.n (absolute value).
        if (op.n == 1 and op.v == 0) {
            return plurality_value::one;
        } else if (op.n >= 2 and op.n <= 4 and op.v == 0) {
            return plurality_value::few;
        } else if (op.v != 0) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::few | plurality_mask::many | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_manx(plural_operand op) noexcept
{
    hilet value = [&] {
        // The specification uses op.i (number of digits) instead of
        // op.n (absolute value).
        if (op.v == 0 and op.n % 10 == 1) {
            return plurality_value::one;
        } else if (op.v == 0 and op.n % 10 == 2) {
            return plurality_value::two;
        } else if (op.v == 0 and (
            op.n % 100 == 0 or op.n % 100 == 20 or op.n % 100 == 40 or op.n % 100 == 60 or op.n % 100 == 80)) {
            return plurality_value::few;
        } else if (op.v != 0) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::two | plurality_mask::few | plurality_mask::many | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_scottish_gaelic(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 1 or op.n == 11) {
            return plurality_value::one;
        } else if (op.n == 2 or op.n == 12) {
            return plurality_value::two;
        } else if ((op.n >= 3 and op.n <= 10) or (op.n >= 13 and op.n <= 19)) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::two | plurality_mask::few | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_breton(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n % 10 == 1 and op.n % 100 != 11 and op.n % 100 != 71 and op.n % 100 != 91) {
            return plurality_value::one;
        } else if (op.n % 10 == 2 and op.n % 100 != 12 and op.n % 100 != 72 and op.n % 100 != 92) {
            return plurality_value::two;
        } else if (
            (op.n % 10 == 3 or op.n % 10 == 4 or op.n % 10 == 9) and
            (not (op.n % 100 >= 10 and op.n % 100 <= 19) and
             not (op.n % 100 >= 70 and op.n % 100 <= 79) and
             not (op.n % 100 >= 90 and op.n % 100 <= 99))) {
            return plurality_value::few;
        } else if (op.n != 0 and op.n % 1'000'000 == 0) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::two | plurality_mask::few | plurality_mask::many | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_lower_sorbian(plural_operand op) noexcept
{
    hilet value = [&] {
        // The specification use op.i (number of digits) instead of
        // op.n (absolute value).
        if (
            (op.v == 0 and op.n % 100 == 1) or
            op.f % 100 == 1) {
            return plurality_value::one;
        } else if (
            (op.v == 0 and op.n % 100 == 2) or
            op.f % 100 == 2) {
            return plurality_value::two;
        } else if (
            (op.v == 0 and op.n % 100 >= 3 and op.n % 100 <= 4) or
            (op.f % 100 >= 3 and op.f % 100 <= 4)) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::two | plurality_mask::few | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_hebrew(plural_operand op) noexcept
{
    hilet value = [&] {
        // The specification use op.i (number of digits) instead of
        // op.n (absolute value).
        if (op.n == 1 and op.v == 0) {
            return plurality_value::one;
        } else if (op.n == 2 and op.v == 0) {
            return plurality_value::two;
        } else if (op.v == 0 and not (op.n >= 0 and op.n <= 10) and op.n % 10 == 0) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::two | plurality_mask::many | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_maltese(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 1) {
            return plurality_value::one;
        } else if (op.n == 0 or (op.n % 100 >= 2 and op.n % 100 <= 10)) {
            return plurality_value::few;
        } else if (op.n % 100 >= 11 and op.n % 100 <= 19) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::few | plurality_mask::many | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_irish(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 1) {
            return plurality_value::one;
        } else if (op.n == 2) {
            return plurality_value::two;
        } else if (op.n >= 3 and op.n <= 6) {
            return plurality_value::few;
        } else if (op.n >= 7 and op.n <= 10) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {value, plurality_mask::one | plurality_mask::two | plurality_mask::few | plurality_mask::many | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_arabic(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 0) {
            return plurality_value::zero;
        } else if (op.n == 1) {
            return plurality_value::one;
        } else if (op.n == 2) {
            return plurality_value::two;
        } else if (op.n % 100 >= 3 and op.n % 100 <= 10) {
            return plurality_value::few;
        } else if (op.n % 100 >= 11 and op.n % 100 <= 99) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {
        value,
        plurality_mask::zero | plurality_mask::one | plurality_mask::two |
        plurality_mask::few | plurality_mask::many | plurality_mask::other};
}

[[nodiscard]] constexpr plurality cardinal_plural_welsh(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 0) {
            return plurality_value::zero;
        } else if (op.n == 1) {
            return plurality_value::one;
        } else if (op.n == 2) {
            return plurality_value::two;
        } else if (op.n == 3) {
            return plurality_value::few;
        } else if (op.n == 6) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {
        value,
        plurality_mask::zero | plurality_mask::one | plurality_mask::two |
        plurality_mask::few | plurality_mask::many | plurality_mask::other};
 }

[[nodiscard]] constexpr plurality cardinal_plural_cornish(plural_operand op) noexcept
{
    hilet value = [&] {
        if (op.n == 0) {
            return plurality_value::zero;
        } else if (op.n == 1) {
            return plurality_value::one;
        } else if (
            op.n % 100 == 2 or
            op.n % 100 == 22 or
            op.n % 100 == 42 or
            op.n % 100 == 62 or
            op.n % 100 == 82 or
            (op.n % 1'000 == 0 and (
                (op.n % 100'000 >= 1'000 and op.n % 100'000 <= 20'000) or
                op.n % 100'000 == 40'000 or
                op.n % 100'000 == 60'000 or
                op.n % 100'000 == 80'000)) or
            (op.n != 0 and op.n % 1'000'000 == 100'000)) {
            return plurality_value::two;
        } else if (
            op.n % 100 == 3 or
            op.n % 100 == 23 or
            op.n % 100 == 43 or
            op.n % 100 == 63 or
            op.n % 100 == 83) {
            return plurality_value::few;
        } else if (op.n != 1 and (
            op.n % 100 == 1 or
            op.n % 100 == 21 or
            op.n % 100 == 41 or
            op.n % 100 == 61 or
            op.n % 100 == 81)) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();

    return {
        value,
        plurality_mask::zero | plurality_mask::one | plurality_mask::two |
        plurality_mask::few | plurality_mask::many | plurality_mask::other};
}
// clang-format on

using cardinal_plural_fptr = plurality (*)(plural_operand);

[[nodiscard]] constexpr auto cardinal_plural_table_init() noexcept
{
    auto r = std::array<cardinal_plural_fptr, 32768>{};

    // Bambara, Burmese, Cantonese, Chinese, Dzongkha, Igbo, Indonesian,
    // Japanese, Javanese, Kabuverdianu, Khmer, Korean, Koyraboro Senni, Lakota,
    // Lao, Lojban, Makonde, Malay, N’Ko, Osage, Root, Sakha, Sango, Sichuan Yi,
    // Sundanese, Thai, Tibetan, Tongan, Vietnamese, Wolof, Yoruba
    for (auto& rule_ptr : r) {
        rule_ptr = cardinal_plural_bambara;
    }

    // Cebuano, Filipino, Tagalog
    r[iso_639{"ceb"}.intrinsic()] = cardinal_plural_cebuano;
    r[iso_639{"fil"}.intrinsic()] = cardinal_plural_cebuano;
    r[iso_639{"tl"}.intrinsic()] = cardinal_plural_cebuano;

    // Central Atlas Tamazight
    r[iso_639{"tzm"}.intrinsic()] = cardinal_plural_central_atlas_tamazight;

    // Icelandic, Macedonian
    r[iso_639{"is"}.intrinsic()] = cardinal_plural_icelandic;
    r[iso_639{"mk"}.intrinsic()] = cardinal_plural_icelandic;

    // Akan, Amharic, Armenian, Assamese, Bangla, Bihari, French, Fulah,
    r[iso_639{"ak"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"am"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"hy"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"as"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"bn"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"bh"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"fr"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"ff"}.intrinsic()] = cardinal_plural_akan;
    // Gujarati, Gun, Hindi, Kabyle, Kannada, Lingala, Malagasy, Northern Sotho,
    r[iso_639{"gu"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"guw"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"hi"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"kab"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"kn"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"ln"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"mg"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"nso"}.intrinsic()] = cardinal_plural_akan;
    // Persian, Portuguese, Punjabi, Sinhala, Tigrinya, Walloon, Zulu
    r[iso_639{"fa"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"pt"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"pa"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"si"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"ti"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"wa"}.intrinsic()] = cardinal_plural_akan;
    r[iso_639{"zu"}.intrinsic()] = cardinal_plural_akan;

    // Afrikaans, Albanian, Aragonese, Asturian, Asu, Azerbaijani, Basque,
    r[iso_639{"af"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"sq"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"an"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ast"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"asa"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"az"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"eu"}.intrinsic()] = cardinal_plural_afrikaans;
    // Bemba, Bena, Bodo, Bulgarian, Catalan, Central Kurdish, Chechen,
    r[iso_639{"bem"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"bez"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"brx"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"bg"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ca"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ckb"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ce"}.intrinsic()] = cardinal_plural_afrikaans;
    // Cherokee, Chiga, Danish, Divehi, Dutch, English, Esperanto, Estonian,
    r[iso_639{"chr"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"cgg"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"da"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"dv"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"nl"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"en"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"eo"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"et"}.intrinsic()] = cardinal_plural_afrikaans;
    // European Portuguese, Ewe, Faroese, Finnish, Friulian, Galician, Ganda,
    //r[iso_639{"pt"}] = cardinal_plural_afrikaans;
    r[iso_639{"ee"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"fo"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"fi"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"fur"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"gl"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"lg"}.intrinsic()] = cardinal_plural_afrikaans;
    // Georgian, German, Greek, Hausa, Hawaiian, Hungarian, Ido, Interlingua,
    r[iso_639{"ka"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"de"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"el"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ha"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"haw"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"hu"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"io"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ia"}.intrinsic()] = cardinal_plural_afrikaans;
    // Italian, Jju, Kako, Kalaallisut, Kashmiri, Kazakh, Kurdish, Kyrgyz,
    r[iso_639{"it"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"kaj"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"kkj"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"kl"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ks"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"kk"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ku"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ky"}.intrinsic()] = cardinal_plural_afrikaans;
    // Luxembourgish, Machame, Malayalam, Marathi, Masai, Metaʼ, Mongolian,
    r[iso_639{"lb"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"jmc"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ml"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"mr"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"mas"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"mgo"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"mn"}.intrinsic()] = cardinal_plural_afrikaans;
    // Nahuatl, Nepali, Ngiemboon, Ngomba, North Ndebele, Norwegian,
    r[iso_639{"nah"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ne"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"nnh"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"jgo"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ns"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"no"}.intrinsic()] = cardinal_plural_afrikaans;
    // Norwegian Bokmål, Norwegian Nynorsk, Nyanja, Nyankole, Odia, Oromo,
    r[iso_639{"nb"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"nn"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ny"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"nyn"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"or"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"om"}.intrinsic()] = cardinal_plural_afrikaans;
    // Ossetic, Papiamento, Pashto, Romansh, Rombo, Rwa, Saho, Samburu,
    r[iso_639{"os"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"pap"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ps"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"rm"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"rof"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"rwk"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ssy"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"saq"}.intrinsic()] = cardinal_plural_afrikaans;
    // Sardinian, Sena, Shambala, Shona, Sicilian, Sindhi, Soga, Somali,
    r[iso_639{"sc"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"seh"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ksb"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"sn"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"scn"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"sd"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"xog"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"so"}.intrinsic()] = cardinal_plural_afrikaans;
    // South Ndebele, Southern Kurdish, Southern Sotho, Spanish, Swahili,
    r[iso_639{"nr"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"sdh"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"st"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"es"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"sw"}.intrinsic()] = cardinal_plural_afrikaans;
    // Swati, Swedish, Swiss German, Syriac, Tamil, Telugu, Teso, Tigre,
    r[iso_639{"ss"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"sv"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"gsw"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"syr"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ta"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"te"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"teo"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"tig"}.intrinsic()] = cardinal_plural_afrikaans;
    // Tsonga, Tswana, Turkish, Turkmen, Tyap, Urdu, Uyghur, Uzbek, Venda,
    r[iso_639{"ts"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"tn"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"tr"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"tk"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"kcg"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ur"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ug"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"uz"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ve"}.intrinsic()] = cardinal_plural_afrikaans;
    // Volapük, Vunjo, Walser, Western Frisian, Xhosa, Yiddish
    r[iso_639{"vo"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"vun"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"wae"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"fy"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"xh"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"ji"}.intrinsic()] = cardinal_plural_afrikaans;
    r[iso_639{"yi"}.intrinsic()] = cardinal_plural_afrikaans; // Yiddish twice.

    // Latvian, Prussian
    r[iso_639{"lv"}.intrinsic()] = cardinal_plural_latvian;
    r[iso_639{"prg"}.intrinsic()] = cardinal_plural_latvian;

    // Colognian, Langi
    r[iso_639{"ksh"}.intrinsic()] = cardinal_plural_colognian;
    r[iso_639{"lag"}.intrinsic()] = cardinal_plural_colognian;

    // Inari Sami, Inuktitut, Lule Sami, Nama, Northern Sami,
    r[iso_639{"smn"}.intrinsic()] = cardinal_plural_inari_sami;
    r[iso_639{"iu"}.intrinsic()] = cardinal_plural_inari_sami;
    r[iso_639{"smj"}.intrinsic()] = cardinal_plural_inari_sami;
    r[iso_639{"naq"}.intrinsic()] = cardinal_plural_inari_sami;
    r[iso_639{"se"}.intrinsic()] = cardinal_plural_inari_sami;
    // Sami languages [Other], Skolt Sami, Southern Sami
    r[iso_639{"smi"}.intrinsic()] = cardinal_plural_inari_sami;
    r[iso_639{"sms"}.intrinsic()] = cardinal_plural_inari_sami;
    r[iso_639{"sma"}.intrinsic()] = cardinal_plural_inari_sami;

    // Belarusian, Russian, Ukrainian
    r[iso_639{"be"}.intrinsic()] = cardinal_plural_belarusian;
    r[iso_639{"ru"}.intrinsic()] = cardinal_plural_belarusian;
    r[iso_639{"uk"}.intrinsic()] = cardinal_plural_belarusian;

    // Polish
    r[iso_639{"pl"}.intrinsic()] = cardinal_plural_polish;

    // Lithuanian
    r[iso_639{"lt"}.intrinsic()] = cardinal_plural_lithuanian;

    // Bosnian, Croatian, Serbian, Serbo-Croatian
    r[iso_639{"bs"}.intrinsic()] = cardinal_plural_bosnian;
    r[iso_639{"hr"}.intrinsic()] = cardinal_plural_bosnian;
    r[iso_639{"sr"}.intrinsic()] = cardinal_plural_bosnian;
    r[iso_639{"sh"}.intrinsic()] = cardinal_plural_bosnian;

    // Tachelhit
    r[iso_639{"shi"}.intrinsic()] = cardinal_plural_tachelhit;

    // Moldavian, Romanian
    r[iso_639{"mo"}.intrinsic()] = cardinal_plural_moldavian;
    r[iso_639{"ro"}.intrinsic()] = cardinal_plural_moldavian;

    // Czech, Slovak
    r[iso_639{"cs"}.intrinsic()] = cardinal_plural_czech;
    r[iso_639{"sk"}.intrinsic()] = cardinal_plural_czech;

    // Manx
    r[iso_639{"gv"}.intrinsic()] = cardinal_plural_manx;

    // Scottish Gaelic
    r[iso_639{"gd"}.intrinsic()] = cardinal_plural_scottish_gaelic;

    // Breton
    r[iso_639{"br"}.intrinsic()] = cardinal_plural_breton;

    // Lower Sorbian, Slovenian, Upper Sorbian
    r[iso_639{"dsb"}.intrinsic()] = cardinal_plural_lower_sorbian;
    r[iso_639{"sl"}.intrinsic()] = cardinal_plural_lower_sorbian;
    r[iso_639{"hsb"}.intrinsic()] = cardinal_plural_lower_sorbian;

    // Hebrew
    r[iso_639{"he"}.intrinsic()] = cardinal_plural_hebrew;

    // Maltese
    r[iso_639{"mt"}.intrinsic()] = cardinal_plural_maltese;

    // Irish
    r[iso_639{"ga"}.intrinsic()] = cardinal_plural_irish;

    // Arabic, Najdi Arabic
    r[iso_639{"ar"}.intrinsic()] = cardinal_plural_arabic;
    r[iso_639{"ars"}.intrinsic()] = cardinal_plural_arabic;

    // Welsh
    r[iso_639{"cy"}.intrinsic()] = cardinal_plural_welsh;

    // Cornish
    r[iso_639{"kw"}.intrinsic()] = cardinal_plural_cornish;

    return r;
}

constexpr auto cardinal_plural_table = cardinal_plural_table_init();

} // namespace detail

/** Get plural information of a number in a given language.
 *
 * @param language The language.
 * @param n The number to know the plurality for.
 * @return plurality information.
 */
[[nodiscard]] constexpr plurality cardinal_plural(language_tag language, std::integral auto n) noexcept
{
    if (language == language_tag{"pt-PT"}) {
        // Portuguese in Portugal is different from Portuguese spoken in
        // other regions.
        return cardinal_plural_afrikaans(detail::plural_operand(n));
    }

    hilet language_index = language.region.intrinsic();
    hi_axiom_bounds(language_index, detail::cardinal_plural_table);
    return detail::cardinal_plural_table[language_index](detail::plural_operand(n));
}

/** Get an index into message plural-variants for a given number.
 *
 * @param language The language the messages are in.
 * @param n The number to format in the message.
 * @param size The number of message plural-variants. Must be larger than 0.
 * @return An index into the plural-variants, if the number of variants is less than plurality suggest then zero is returned.
 */
[[nodiscard]] constexpr size_t cardinal_plural(language_tag language, std::integral auto n, size_t size) noexcept
{
    return cardinal_plural(language, n).index(size);
}

}} // namespace hi::v1

