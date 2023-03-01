
#include "iso_639.hpp"
#include "../utlility/module.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>

namespace hi { inline namespace v1 {

enum class plurality_value : uint8_t { zero = 0, one = 1, two = 2, few = 3, many = 4, other = 5 };

enum class plurality_mask : uint8_t {
    zero = 1 << to_underlying(plurality::zero),
    one = 1 << to_underlying(plurality::one),
    two = 1 << to_underlying(plurality::two),
    few = 1 << to_underlying(plurality::few),
    many = 1 << to_underlying(plurality::many),
    other = 1 << to_underlying(plurality::other),
};

[[nodiscard]] constexpr plurality_mask operator|(plurality_mask const& lhs, plurality_mask const& rhs) noexcept
{
    return static_cast<plurality_mask>(to_underlying(lhs) | to_underlying(rhs));
}

struct plurality {
    plurality_value value;
    plurality_mask mask;

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

        hilet value_as_mask = (1 << (to_underlying(value) + 1)) - 1;
        // Get the index based on the number of '1' bits that are set from the
        // plurality position to lsb.
        hilet i = std::popcount(value_as_mask & to_underlying(mask)) - 1;
        if (i < n) {
            return i;
        } else {
            return n - 1;
        }
    }
};

/** Calculate the plurality of a value.
 *
 * @param i An integer to format.
 * @return plurality, mask of possible pluralities for this language.
 */
using plurality_func_ptr = plurality (*)(long long i);

// The following plurality rules are named by the first language in the alphabet
// which has this plurality rule.

[[nodiscard]] plurality plurality_azerbijani(long long n) noexcept
{
    return {plurality_value::other, plurality_mask::other};
}

[[nodiscard]] plurality plurality_manx(long long n) noexcept
{
    hilet value = [&] {
        if (n % 10 == 1 or n % 10 == 2 or n % 20 == 0) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_central_atlas_tamazight(long long n) noexcept
{
    hilet value = [&] {
        if (n == 0 or n == 1 or (n >= 11 and n <= 99) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_macedonian(long long n) noexcept
{
    hilet value = [&] {
        if (n % 10 == 1 and n != 11) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_akan(long long n) noexcept
{
    hilet value = [&] {
        if (n == 0 or n == 1) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();
    return {value, plurality_mask::one | plurality_mask::other};
}

[[nodiscard]] plurality plurality_afrikaans(long long n) noexcept
{
    hilet value = [&] {
        if (n == 1) {
            return plurality_value::one;
        } else {
            return plurality_value::other;
        }
    }();
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
