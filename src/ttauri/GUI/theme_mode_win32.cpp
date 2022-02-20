// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme_mode.hpp"
#include "../strings.hpp"
#include "../log.hpp"
#include "../registry_win32.hpp"

namespace tt::inline v1 {

[[nodiscard]] theme_mode read_os_theme_mode() noexcept
{
    try {
        auto result = registry_read_current_user_dword(
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", "AppsUseLightTheme");
        return result ? theme_mode::light : theme_mode::dark;
    } catch (...) {
        return theme_mode::light;
    }
}

} // namespace tt::inline v1
