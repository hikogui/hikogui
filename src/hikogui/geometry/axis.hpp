// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hi::inline v1 {

enum class axis : unsigned char {
    none = 0,
    x = 1,
    y = 2,
    z = 4,
    both = x | y,
    all = x | y | z,

    row = x,
    column = y,

    horizontal = x,
    vertical = y,
};

[[nodiscard]] constexpr axis operator&(axis const &lhs, axis const &rhs) noexcept
{
    return static_cast<axis>(static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs));
}

[[nodiscard]] constexpr axis operator|(axis const &lhs, axis const &rhs) noexcept
{
    return static_cast<axis>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
}

[[nodiscard]] constexpr bool any(axis const &rhs) noexcept
{
    return to_bool(static_cast<unsigned char>(rhs));
}

} // namespace hi::inline v1
