// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../assert.hpp"
#include <string>
#include <ostream>

namespace tt::inline v1 {

enum class theme_mode { light, dark };

[[nodiscard]] constexpr char const *to_const_string(theme_mode rhs) noexcept
{
    switch (rhs) {
    case theme_mode::light: return "light";
    case theme_mode::dark: return "dark";
    default: tt_no_default();
    }
}

[[nodiscard]] inline std::string to_string(theme_mode rhs) noexcept
{
    return to_const_string(rhs);
}

inline std::ostream &operator<<(std::ostream &lhs, theme_mode rhs)
{
    return lhs << to_const_string(rhs);
}

theme_mode read_os_theme_mode() noexcept;

} // namespace tt::inline v1

template<typename CharT>
struct std::formatter<tt::theme_mode, CharT> : std::formatter<char const *, CharT> {
    auto format(tt::theme_mode const &t, auto &fc)
    {
        return std::formatter<char const *, CharT>::format(tt::to_const_string(t), fc);
    }
};
