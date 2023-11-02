// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <string>
#include <ostream>

hi_export_module(hikogui.settings.theme_mode);

hi_export namespace hi::inline v1 {

enum class theme_mode { light, dark };

constexpr auto theme_mode_metadata = enum_metadata{theme_mode::light, "light", theme_mode::dark, "dark"};

[[nodiscard]] hi_inline std::string_view to_string(theme_mode rhs) noexcept
{
    return theme_mode_metadata[rhs];
}

hi_inline std::ostream &operator<<(std::ostream &lhs, theme_mode rhs)
{
    return lhs << theme_mode_metadata[rhs];
}

} // namespace hi::inline v1

// XXX #617 MSVC bug does not handle partial specialization in modules.
hi_export template<>
struct std::formatter<hi::theme_mode, char> : std::formatter<std::string_view, char> {
    auto format(hi::theme_mode const &t, auto &fc) const
    {
        return std::formatter<std::string_view, char>::format(hi::theme_mode_metadata[t], fc);
    }
};
