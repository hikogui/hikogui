// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "language.hpp"
#include "../required.hpp"
#include "../log.hpp"
#include <Windows.h>
#include <Winnls.h>

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
std::vector<language_tag> language::read_os_preferred_languages() noexcept
{
    ttlet subkey = tt::to_wstring("Control Panel\\International\\User Profile");
    ttlet name = tt::to_wstring("Languages");

    wchar_t result[256];
    DWORD result_length = sizeof(result);

    auto status =
        RegGetValueW(HKEY_CURRENT_USER, subkey.c_str(), name.c_str(), RRF_RT_REG_MULTI_SZ, NULL, &result, &result_length);

    switch (status) {
    case ERROR_SUCCESS: {
        ttlet language_names = ZZWSTR_to_string(result, result + result_length);
        auto r = std::vector<language_tag>{};
        r.reserve(size(language_names));
        for (ttlet &language_name : language_names) {
            r.emplace_back(language_name);
        }
        return r;
    }

    case ERROR_BAD_PATHNAME:
    case ERROR_FILE_NOT_FOUND: {
        auto reg_path = "HKEY_CURRENT_USER\\" + tt::to_string(subkey) + "\\" + tt::to_string(name);
        tt_log_error("Missing {} registry entry: 0x{:08x}", reg_path, status);
        return {language_tag{"en"}};
    }

    default:
        auto reg_path = "HKEY_CURRENT_USER\\" + tt::to_string(subkey) + "\\" + tt::to_string(name);
        tt_log_error("Unknown error when getting {} registry value. {:08x}", reg_path, status);
        return {language_tag{"en"}};
    }
}

} // namespace tt::inline v1
