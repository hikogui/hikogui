// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/ThemeMode.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/logger.hpp"
#define WIN32_NO_STATUS 1
#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <winreg.h>
#include <Uxtheme.h>


namespace TTauri {

[[nodiscard]] ThemeMode readOSThemeMode() noexcept
{
    let subkey = TTauri::to_wstring("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize");
    let name = TTauri::to_wstring("AppsUseLightTheme");
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
            return ThemeMode::Light;
        } else {
            return ThemeMode::Dark;
        }

    case ERROR_BAD_PATHNAME:
    case ERROR_FILE_NOT_FOUND: {
        auto reg_path = "HKEY_CURRENT_USER\\" + TTauri::to_string(subkey) + "\\" + TTauri::to_string(name);

        LOG_ERROR("Missing {} registry entry: 0x{:08x}", reg_path, status);
        } return ThemeMode::Light;

    default:
        LOG_FATAL("Could get AppsUseLightTheme registry value. {:08x}", status);
    }

}

}