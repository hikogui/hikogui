// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme_mode.hpp"
#include "../strings.hpp"
#include "../log.hpp"
#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <winreg.h>
#include <Uxtheme.h>


namespace tt {
inline namespace v1 {

[[nodiscard]] theme_mode read_os_theme_mode() noexcept
{
    ttlet subkey = tt::to_wstring("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize");
    ttlet name = tt::to_wstring("AppsUseLightTheme");
    DWORD result;
    DWORD result_length = sizeof(result);
    auto status = RegGetValueW(
        HKEY_CURRENT_USER,
        subkey.c_str(),
        name.c_str(),
        RRF_RT_DWORD,
        NULL,
        &result,
        &result_length
    );

    switch (status) {
    case ERROR_SUCCESS:
        if (result) {
            return theme_mode::light;
        } else {
            return theme_mode::dark;
        }

    case ERROR_BAD_PATHNAME:
    case ERROR_FILE_NOT_FOUND: {
        auto reg_path = "HKEY_CURRENT_USER\\" + tt::to_string(subkey) + "\\" + tt::to_string(name);

        tt_log_error("Missing {} registry entry: 0x{:08x}", reg_path, status);
        } return theme_mode::light;

    default:
        tt_log_fatal("Could get AppsUseLightTheme registry value. {:08x}", status);
    }

}

}
}
