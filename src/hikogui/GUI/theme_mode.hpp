// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility.hpp"
#include "../assert.hpp"
#include "../enum_metadata.hpp"
#include <string>
#include <ostream>

namespace hi::inline v1 {

enum class theme_mode { light, dark };

constexpr auto theme_mode_metadata = enum_metadata{theme_mode::light, "light", theme_mode::dark, "dark"};

[[nodiscard]] inline std::string_view to_string(theme_mode rhs) noexcept
{
    return theme_mode_metadata[rhs];
}

inline std::ostream &operator<<(std::ostream &lhs, theme_mode rhs)
{
    return lhs << theme_mode_metadata[rhs];
}

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::theme_mode, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::theme_mode const &t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(hi::theme_mode_metadata[t], fc);
    }
};
