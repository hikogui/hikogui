// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "os_settings.hpp"
#include "registry_win32.hpp"
#include "log.hpp"
#include "cast.hpp"
#include <windows.h>
#undef WIN32_NO_STATUS
#include <winuser.h>

namespace tt::inline v1 {

/**
 * GetUserPreferredUILanguages() returns at most two of the selected languages in random order
 * and can not be used to retrieve the preferred languages the user has selected.
 *
 * The winrt GlobalizationPreferences::Languages returns all languages in the correct order.
 * However winrt header files are incompatible with c++20 co-routines.
 *
 * Therefor the only option available is to read the language list from the registry.
 */
[[nodiscard]] std::vector<language_tag> os_settings::gather_languages() noexcept
{
    try {
        ttlet strings = registry_read_current_user_multi_string("Control Panel\\International\\User Profile", "Languages");

        auto r = std::vector<language_tag>{};
        r.reserve(strings.size());
        for (ttlet &string : strings) {
            r.push_back(language_tag{string});
        }
        return r;

    } catch (...) {
        return {language_tag{"en"}};
    }
}

[[nodiscard]] tt::theme_mode os_settings::gather_theme_mode() noexcept
{
    try {
        auto result = registry_read_current_user_dword(
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", "AppsUseLightTheme");
        return result ? theme_mode::light : theme_mode::dark;
    } catch (...) {
        return theme_mode::light;
    }
}

[[nodiscard]] std::chrono::milliseconds os_settings::gather_double_click_interval() noexcept
{
    return std::chrono::milliseconds{GetDoubleClickTime()};
}

[[nodiscard]] std::chrono::milliseconds os_settings::gather_keyboard_repeat_delay() noexcept
{
    using namespace std::literals::chrono_literals;

    INT r;
    if (not SystemParametersInfoW(SPI_GETKEYBOARDDELAY, 0, &r, 0)) {
        tt_log_error("Could not get system parameter SPI_GETKEYBOARDDELAY: {}", get_last_error_message());
        return std::chrono::milliseconds{250};
    }

    // SPI_GETKEYBOARDDELAY values are between 0 (250ms) to 3 (1s).
    auto bias = 250ms;
    auto gain = 250ms;

    return bias + r * gain;
}

[[nodiscard]] std::chrono::milliseconds os_settings::gather_keyboard_repeat_interval() noexcept
{
    using namespace std::literals::chrono_literals;

    INT r;
    if (not SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0, &r, 0)) {
        tt_log_error("Could not get system parameter SPI_GETKEYBOARDSPEED: {}", get_last_error_message());
        return std::chrono::milliseconds{100};
    }

    // SPI_GETKEYBOARDSPEED values are between 0 (2.5 per/sec) to 31 (30 per/sec).
    auto bias = 2.5f;
    auto gain = 0.887f;
    auto rate = bias + r * gain;
    return std::chrono::duration_cast<std::chrono::milliseconds>(1000ms / rate);
}

[[nodiscard]] std::chrono::milliseconds os_settings::gather_cursor_blink_interval() noexcept
{
    using namespace std::literals::chrono_literals;

    auto r = GetCaretBlinkTime();
    if (r == 0) {
        tt_log_error("Could not get caret blink time: {}", get_last_error_message());
        return std::chrono::milliseconds{1000};
    }

    if (r == INFINITE) {
        return std::chrono::milliseconds::max();
    } else {
        // GetGaretBlinkTime() gives the time for a half-period.
        return std::chrono::milliseconds{r} * 2;
    }
}

[[nodiscard]] std::chrono::milliseconds os_settings::gather_cursor_blink_delay() noexcept
{
    // The blink delay is not available in the OS, we can use the keyboard repeat delay.
    return std::max(gather_keyboard_repeat_delay(), gather_keyboard_repeat_interval());
}

[[nodiscard]] extent2 os_settings::gather_minimum_window_size() noexcept
{
    auto width = GetSystemMetrics(SM_CXMINTRACK);
    if (width == 0) {
        tt_log_error("Could not retrieve SM_CXMINTRACK");
        width = 40;
    }

    auto height = GetSystemMetrics(SM_CYMINTRACK);
    if (height == 0) {
        tt_log_error("Could not retrieve SM_CYMINTRACK");
        height = 25;
    }
    return extent2{narrow<float>(width), narrow<float>(height)};
}

[[nodiscard]] extent2 os_settings::gather_maximum_window_size() noexcept
{
    auto width = GetSystemMetrics(SM_CXMAXTRACK);
    if (width == 0) {
        tt_log_error("Could not retrieve SM_CXMAXTRACK");
        width = 1920;
    }

    auto height = GetSystemMetrics(SM_CYMAXTRACK);
    if (height == 0) {
        tt_log_error("Could not retrieve SM_CYMAXTRACK");
        height = 1080;
    }
    return extent2{narrow<float>(width), narrow<float>(height)};
}

} // namespace tt::inline v1
