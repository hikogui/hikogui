// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "win32_headers.hpp"

#include "os_settings.hpp"
#include "registry_win32.hpp"
#include "log.hpp"
#include "cast.hpp"
#include "exception.hpp"

namespace hi::inline v1 {

/**
 * GetUserPreferredUILanguages() returns at most two of the selected languages in random order
 * and can not be used to retrieve the preferred languages the user has selected.
 *
 * The winrt GlobalizationPreferences::Languages returns all languages in the correct order.
 * However winrt header files are incompatible with c++20 co-routines.
 *
 * Therefor the only option available is to read the language list from the registry.
 */
[[nodiscard]] std::vector<language_tag> os_settings::gather_languages()
{
    hilet strings = registry_read_current_user_multi_string("Control Panel\\International\\User Profile", "Languages");

    auto r = std::vector<language_tag>{};
    r.reserve(strings.size());
    for (hilet& string : strings) {
        r.push_back(language_tag{string});
    }
    return r;
}

[[nodiscard]] hi::theme_mode os_settings::gather_theme_mode()
{
    try {
        hilet result = registry_read_current_user_dword(
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", "AppsUseLightTheme");
        return result ? theme_mode::light : theme_mode::dark;
    } catch (...) {
        return theme_mode::light;
    }
}

[[nodiscard]] hi::subpixel_orientation os_settings::gather_subpixel_orientation()
{
    {
        BOOL has_font_smoothing;
        if (not SystemParametersInfoW(SPI_GETFONTSMOOTHING, 0, &has_font_smoothing, 0)) {
            throw os_error(std::format("Could not get system parameter SPI_GETFONTSMOOTHING: {}", get_last_error_message()));
        }

        if (has_font_smoothing == FALSE) {
            // Font smoothing is disabled.
            return hi::subpixel_orientation::unknown;
        }
    }

    {
        UINT font_smooth_type;
        if (not SystemParametersInfoW(SPI_GETFONTSMOOTHINGTYPE, 0, &font_smooth_type, 0)) {
            throw os_error(std::format("Could not get system parameter SPI_GETFONTSMOOTHINGTYPE: {}", get_last_error_message()));
        }

        if (font_smooth_type != FE_FONTSMOOTHINGCLEARTYPE) {
            // Font smoothing is not clear type.
            return hi::subpixel_orientation::unknown;
        }
    }

    {
        BOOL has_clear_type;
        if (not SystemParametersInfoW(SPI_GETCLEARTYPE, 0, &has_clear_type, 0)) {
            throw os_error(std::format("Could not get system parameter SPI_GETCLEARTYPE: {}", get_last_error_message()));
        }

        if (has_clear_type == FALSE) {
            // ClearType is disabled.
            return hi::subpixel_orientation::unknown;
        }
    }

    {
        UINT font_smooth_orientation;
        if (not SystemParametersInfoW(SPI_GETFONTSMOOTHINGORIENTATION, 0, &font_smooth_orientation, 0)) {
            throw os_error(
                std::format("Could not get system parameter SPI_GETFONTSMOOTHINGORIENTATION: {}", get_last_error_message()));
        }

        if (font_smooth_orientation == FE_FONTSMOOTHINGORIENTATIONBGR) {
            // Font smoothing is not clear type.
            return hi::subpixel_orientation::horizontal_bgr;
        } else if (font_smooth_orientation == FE_FONTSMOOTHINGORIENTATIONRGB) {
            // Font smoothing is not clear type.
            return hi::subpixel_orientation::horizontal_rgb;
        } else {
            throw os_error(std::format("Unknown result from SPI_GETFONTSMOOTHINGORIENTATION: {}", font_smooth_orientation));
        }
    }
}

[[nodiscard]] bool os_settings::gather_uniform_HDR()
{
    // Microsoft Windows 10 switches display mode when getting a HDR surface
    // The switching causes all application to display using a different color and brightness calibration.
    return false;
}

[[nodiscard]] std::chrono::milliseconds os_settings::gather_double_click_interval()
{
    return std::chrono::milliseconds{GetDoubleClickTime()};
}

[[nodiscard]] float os_settings::gather_double_click_distance()
{
    hilet width = GetSystemMetrics(SM_CXDOUBLECLK);
    if (width <= 0) {
        throw os_error("Could not retrieve SM_CXDOUBLECLK");
    }

    hilet height = GetSystemMetrics(SM_CYDOUBLECLK);
    if (height <= 0) {
        throw os_error("Could not retrieve SM_CYDOUBLECLK");
    }

    hilet diameter = std::max(width, height);
    return diameter * 0.5f;
}

[[nodiscard]] std::chrono::milliseconds os_settings::gather_keyboard_repeat_delay()
{
    using namespace std::literals::chrono_literals;

    INT r;
    if (not SystemParametersInfoW(SPI_GETKEYBOARDDELAY, 0, &r, 0)) {
        throw os_error(std::format("Could not get system parameter SPI_GETKEYBOARDDELAY: {}", get_last_error_message()));
    }

    // SPI_GETKEYBOARDDELAY values are between 0 (250ms) to 3 (1s).
    constexpr auto bias = 250ms;
    constexpr auto gain = 250ms;

    return bias + r * gain;
}

[[nodiscard]] std::chrono::milliseconds os_settings::gather_keyboard_repeat_interval()
{
    using namespace std::literals::chrono_literals;

    INT r;
    if (not SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0, &r, 0)) {
        throw os_error(std::format("Could not get system parameter SPI_GETKEYBOARDSPEED: {}", get_last_error_message()));
    }

    // SPI_GETKEYBOARDSPEED values are between 0 (2.5 per/sec) to 31 (30 per/sec).
    constexpr auto bias = 2.5f;
    constexpr auto gain = 0.887f;
    hilet rate = bias + r * gain;
    return std::chrono::duration_cast<std::chrono::milliseconds>(1000ms / rate);
}

[[nodiscard]] std::chrono::milliseconds os_settings::gather_cursor_blink_interval()
{
    using namespace std::literals::chrono_literals;

    hilet r = GetCaretBlinkTime();
    if (r == 0) {
        throw os_error(std::format("Could not get caret blink time: {}", get_last_error_message()));

    } else if (r == INFINITE) {
        return std::chrono::milliseconds::max();

    } else {
        // GetGaretBlinkTime() gives the time for a half-period.
        return std::chrono::milliseconds{r} * 2;
    }
}

[[nodiscard]] std::chrono::milliseconds os_settings::gather_cursor_blink_delay()
{
    // The blink delay is not available in the OS, we can use the keyboard repeat delay.
    return std::max(gather_keyboard_repeat_delay(), gather_keyboard_repeat_interval());
}

[[nodiscard]] extent2 os_settings::gather_minimum_window_size()
{
    hilet width = GetSystemMetrics(SM_CXMINTRACK);
    if (width == 0) {
        throw os_error("Could not retrieve SM_CXMINTRACK");
    }

    hilet height = GetSystemMetrics(SM_CYMINTRACK);
    if (height == 0) {
        throw os_error("Could not retrieve SM_CYMINTRACK");
    }

    return extent2{narrow_cast<float>(width), narrow_cast<float>(height)};
}

[[nodiscard]] extent2 os_settings::gather_maximum_window_size()
{
    hilet width = GetSystemMetrics(SM_CXMAXTRACK);
    if (width == 0) {
        throw os_error("Could not retrieve SM_CXMAXTRACK");
    }

    hilet height = GetSystemMetrics(SM_CYMAXTRACK);
    if (height == 0) {
        throw os_error("Could not retrieve SM_CYMAXTRACK");
    }

    return extent2{narrow_cast<float>(width), narrow_cast<float>(height)};
}

[[nodiscard]] uintptr_t os_settings::gather_primary_monitor_id()
{
    hilet origin = POINT{0, 0};
    hilet monitor = MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY);
    return std::bit_cast<uintptr_t>(monitor);
}

[[nodiscard]] aarectangle os_settings::gather_primary_monitor_rectangle()
{
    hilet width = GetSystemMetrics(SM_CXSCREEN);
    if (width == 0) {
        throw os_error("Could not retrieve SM_CXSCREEN");
    }

    hilet height = GetSystemMetrics(SM_CYSCREEN);
    if (height == 0) {
        throw os_error("Could not retrieve SM_CYSCREEN");
    }

    // The origin of the primary monitor is also the origin of the desktop.
    return aarectangle{extent2{narrow_cast<float>(width), narrow_cast<float>(height)}};
}

[[nodiscard]] aarectangle os_settings::gather_desktop_rectangle()
{
    hilet primary_monitor_height = GetSystemMetrics(SM_CYSCREEN);
    if (primary_monitor_height == 0) {
        throw os_error("Could not retrieve SM_CYSCREEN");
    }

    hilet left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    hilet top = GetSystemMetrics(SM_YVIRTUALSCREEN);

    hilet width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    if (width == 0) {
        throw os_error("Could not retrieve SM_CXVIRTUALSCREEN");
    }

    hilet height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (height == 0) {
        throw os_error("Could not retrieve SM_CYVIRTUALSCREEN");
    }

    hilet bottom = top + height;

    // Calculate the bottom as compared to a y-axis up coordinate system.
    hilet inv_bottom = primary_monitor_height - bottom; // 0, 600
    return aarectangle{
        narrow_cast<float>(left), narrow_cast<float>(inv_bottom), narrow_cast<float>(width), narrow_cast<float>(height)};
}

} // namespace hi::inline v1
