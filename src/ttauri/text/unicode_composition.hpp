// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../assert.hpp"

namespace tt::inline v1 {

class unicode_composition {
public:
    [[nodiscard]] constexpr unicode_composition(char32_t first, char32_t second, char32_t composed = 0) noexcept :
        value(static_cast<int64_t>(first) << 42 | static_cast<int64_t>(second) << 21 | static_cast<int64_t>(composed))
    {
        tt_axiom(first <= 0x10'ffff);
        tt_axiom(second <= 0x10'ffff);
        tt_axiom(composed <= 0x10'ffff);
    }

    [[nodiscard]] constexpr char32_t first() const noexcept
    {
        return static_cast<char32_t>(value >> 42);
    }

    [[nodiscard]] constexpr char32_t second() const noexcept
    {
        return static_cast<char32_t>((value >> 21) & 0x1f'ffff);
    }

    [[nodiscard]] constexpr char32_t composed() const noexcept
    {
        return static_cast<char32_t>(value & 0x1f'ffff);
    }


    [[nodiscard]] constexpr friend bool operator==(unicode_composition const &lhs, unicode_composition const &rhs) noexcept
    {
        // Don't check the composed value, when searching through the composition table.
        return (lhs.value >> 21) == (rhs.value >> 21);
    }

    [[nodiscard]] constexpr friend bool operator<(unicode_composition const &lhs, unicode_composition const &rhs) noexcept
    {
        return lhs.value < rhs.value;
    }

private:
    int64_t value;
};

template<typename It>
[[nodiscard]] constexpr It unicode_composition_find(It first, It last, unicode_composition value) noexcept
{
    auto it = std::lower_bound(first, last, value);
    if (it == last || *it != value) {
        return last;
    } else {
        return it;
    }
}

template<typename It>
[[nodiscard]] constexpr It unicode_composition_find(It first, It last, char32_t first_cp, char32_t second_cp) noexcept
{
    return unicode_composition_find(first, last, unicode_composition{first_cp, second_cp});
}

/** Find a composition of two code-points.
 * @return The combined character or 0xffff.
 */
[[nodiscard]] char32_t unicode_composition_find(char32_t first, char32_t second) noexcept;

} // namespace tt::inline v1
