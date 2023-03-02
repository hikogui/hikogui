
#include "iso_639.hpp"
#include "../utlility/module.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>

namespace hi { inline namespace v1 {


/** Calculate the plurality of a value.
 *
 * @param i An integer to format.
 * @return plurality, mask of possible pluralities for this language.
 */
using plurality_func_ptr = plurality (*)(long long i);

// The following plurality rules are named by the first language in the alphabet
// which has this plurality rule.
// http://www.unicode.org/cldr/cldr-aux/charts/37/supplemental/language_plural_rules.html

// clang-format off
[[nodiscard]] plurality plurality_bambara(long long n) noexcept
{
    return {plurality_value::other, plurality_mask::other};
}

[[nodiscard]] plurality plurality_cebuano(long long n) noexcept
{
    hilet value = [](auto op) {
        if (
            (op.v == 0 and op.i >= 1 and op.i <= 3) or
            (op.v == 0 and op.i % 10 != 4 and op.i % 10 != 6 and op.i % 10 != 9) or
            (op.v != 0 and op.f % 10 != 4 and op.f % 10 != 6 and op.f % 10 != 9)
        ) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }(plurality_operand(n));

    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_central_atlas_tamazight(long long n) noexcept
{
    hilet value = [](auto op) {
        if (op.n == 0 or op.n == 1 or (op.n >= 11 and op.n <= 99) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }(plurality_operand(n));
    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_icelandic(long long n) noexcept
{
    hilet value = [](auto op) {
        if ((op.t == 0 and op.i % 10 == 1 and op.i % 100 != 11) or op.t != 0) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }(plurality_operand(n));
    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_akan(long long n) noexcept
{
    hilet value = [](auto op) {
        if (op.n == 0 or op.n == 1) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }(plurality_operand(n));
    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_afrikaans(long long n) noexcept
{
    hilet value = [](auto op) {
        if (op.n == 1) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }(plurality_operand(n));
    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_latvian(long long n) noexcept
{
    hilet value = [&] {
        if (n == 0) {
            return plurality_value::zero;
        } else if (n % 10 == 1 and n % 100 != 11) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plural_mask::zero | plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_colognian(long long n) noexcept
{
    hilet value = [&] {
        if (n == 0) {
            return plurality_value::zero;
        } else if (n == 1) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plural_mask::zero | plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_cormish(long long n) noexcept
{
    hilet value = [&] {
        if (n == 1) {
            return plurality_value::one;
        } else if (n == 2) {
            return plurality_value::two;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plural_mask::two | plurality_mask::other};
}

[[nodiscard]] plurality plurality_belarusian(long long n) noexcept
{
    hilet value = [&] {
        if (n % 10 == 1 and n % 100 != 11) {
            return plurality_value::one;
        } else if ((n % 10 >= 2 and n % 10 <= 4) and not(n % 100 >= 12 and n % 100 <= 14)) {
            return plurality_value::few;
        } else if ((n % 10 == 0 or (n % 10 >= 5 and n % 10 <= 9) or (n % 100 >= 11 and n % 100 <= 14)) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plural_mask::few | plural_mask::many | plurality_mask::other};
}

[[nodiscard]] plurality plurality_polish(long long n) noexcept
{
    hilet value = [&] {
        if (n == 1) {
            return plurality_value::one;
        } else if ((n % 10 >= 2 and n % 10 <= 4) and not(n % 100 >= 12 and n % 100 <= 14)) {
            return plurality_value::few;
        } else if (
            (n != 1 and n % 10 >= 0 and n % 10 <= 1) or (n % 10 >= 5 and n % 10 <= 9) or (n % 100 >= 12 and n % 100 <= 14)) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plural_mask::few | plural_mask::many | plurality_mask::other};
}

[[nodiscard]] plurality plurality_lithuanian(long long n) noexcept
{
    hilet value = [&] {
        if (n % 10 == 1 and not(n % 100 >= 11 and n % 100 <= 19)) {
            return plurality_value::one;
        } else if (n % 10 >= 2 and n % 10 <= 9 and not(n % 100 >= 11 and n % 100 <= 19)) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plural_mask::few | plurality_mask::other};
}

[[nodiscard]] plurality plurality_tachelhit(long long n) noexcept
{
    hilet value = [&] {
        if (n == 0 or n == 1) {
            return plurality_value::one;
        } else if (n >= 2 and n <= 10) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plural_mask::few | plurality_mask::other};
}

[[nodiscard]] plurality plurality_moldavian(long long n) noexcept
{
    hilet value = [&] {
        if (n == 1) {
            return plurality_value::one;
        } else if (n == 0 or (n != 1 and n % 100 >= 1 and n % 100 <= 19)) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plural_mask::few | plurality_mask::other};
}

[[nodiscard]] plurality plurality_czech(long long n) noexcept
{
    hilet value = [&] {
        if (n == 1) {
            return plurality_value::one;
        } else if (n >= 2 and n <= 4) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plural_mask::few | plurality_mask::other};
}

[[nodiscard]] plurality plurality_scottish_gaelic(long long n) noexcept
{
    hilet value = [&] {
        if (n == 1 or n == 1) {
            return plurality_value::one;
        } else if (n == 2 or n == 12) {
            return plurality_value::two;
        } else if ((n >= 3 and n <= 10) or (n >= 13 and n <= 19)) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plurality_mask::two | plural_mask::few | plurality_mask::other};
}

[[nodiscard]] plurality plurality_breton(long long n) noexcept
{
    hilet value = [&] {
        if (n % 10 == 1 and n % 100 != 11 and n % 100 != 71 and n % 100 != 91) {
            return plurality_value::one;
        } else if (n % 10 == 2 and n % 100 != 12 and n % 100 != 72 and n % 100 != 92) {
            return plurality_value::two;
        } else if (
            (n % 10 == 3 or n % 10 == 4 or n % 10 == 9) and not(n % 100 >= 10 and n % 100 <= 19) and
            not(n % 100 >= 70 and n % 100 <= 79) and not(n % 100 >= 90 and n % 100 <= 99)) {
            return plurality_value::few;
        } else if (n != 0 and n % 1'000'000 == 0) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plurality_mask::two | plural_mask::few | plural_mask::many | plurality_mask::other};
}

[[nodiscard]] plurality plurality_slovenian(long long n) noexcept
{
    hilet value = [&] {
        if (n % 100 == 1) {
            return plurality_value::one;
        } else if (n % 100 == 2) {
            return plurality_value::two;
        } else if (n % 100 == 3 or n % 100 == 4) {
            return plurality_value::few;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plurality_mask::two | plural_mask::few | plurality_mask::other};
}

[[nodiscard]] plurality plurality_hebrew(long long n) noexcept
{
    hilet value = [&] {
        if (n == 1) {
            return plurality_value::one;
        } else if (n == 2) {
            return plurality_value::two;
        } else if (n != 0 and n % 10 == 0) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plurality_mask::two | plural_mask::many | plurality_mask::other};
}

[[nodiscard]] plurality plurality_maltese(long long n) noexcept
{
    hilet value = [&] {
        if (n == 1) {
            return plurality_value::one;
        } else if (n == 0 or (n % 100 >= 2 and n % 100 <= 10)) {
            return plurality_value::few;
        } else if (n % 100 >= 11 and n % 100 <= 19) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plurality_mask::few | plural_mask::many | plurality_mask::other};
}

[[nodiscard]] plurality plurality_irish(long long n) noexcept
{
    hilet value = [&] {
        if (n == 1) {
            return plurality_value::one;
        } else if (n == 2) {
            return plurality_value::two;
        } else if (n >= 3 and n <= 6) {
            return plurality_value::few;
        } else if (n >= 7 and n <= 10) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plurality_mask::two | plurality_mask::few | plural_mask::many | plurality_mask::other};
}

[[nodiscard]] plurality plurality_arabic(long long n) noexcept
{
    hilet value = [&] {
        if (n == 0) {
            return plurality_value::zero;
        } else if (n == 1) {
            return plurality_value::one;
        } else if (n == 2) {
            return plurality_value::two;
        } else if (n % 100 >= 3 and n % 100 <= 10) {
            return plurality_value::few;
        } else if (n % 100 >= 11 and n % 100 <= 99) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();
    return {
        value,
        plurality_mask::zero | plurality_mask::one | plurality_mask::two | plurality_mask::few | plural_mask::many |
            plurality_mask::other};
}

[[nodiscard]] plurality plurality_welsh(long long n) noexcept
{
    hilet value = [&] {
        if (n == 0) {
            return plurality_value::zero;
        } else if (n == 1) {
            return plurality_value::one;
        } else if (n == 2) {
            return plurality_value::two;
        } else if (n == 3) {
            return plurality_value::few;
        } else if (n == 6) {
            return plurality_value::many;
        } else {
            return plurality_value::other;
        }
    }();
    return {
        value,
        plurality_mask::zero | plurality_mask::one | plurality_mask::two | plurality_mask::few | plural_mask::many |
            plurality_mask::other};
}
// clang-format off

[[nodiscard]] constexpr auto init_plurality_func_ptr() noexcept
{
    auto r = std::array<plurality_func_ptr, 32768>{};
    for (auto i = 0_uz; i != r.size(); ++i) {
        // This is basically a null implementation of plurality rules.
        r[i] = plurality_azerbaijani();
    }

    r[iso_639{"af"}.intrinsic()] = plurality_afrikaans;

    return r;
}

constexpr auto plurality_func_ptr = init_plurality_func_ptr();

}} // namespace hi::v1
