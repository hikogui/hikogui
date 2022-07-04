// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "registry_win32.hpp"
#include "exception.hpp"
#include "strings.hpp"
#include "log.hpp"
#include "win32_headers.hpp"

namespace hi::inline v1 {

[[nodiscard]] uint32_t registry_read_current_user_dword(std::string_view path, std::string_view name)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);

    DWORD result;
    DWORD result_length = sizeof(result);
    hilet status = RegGetValueW(HKEY_CURRENT_USER, wpath.c_str(), wname.c_str(), RRF_RT_DWORD, NULL, &result, &result_length);

    switch (status) {
    case ERROR_SUCCESS: break;
    case ERROR_BAD_PATHNAME:
    case ERROR_FILE_NOT_FOUND:
        throw os_error(std::format("Missing HKEY_CURRENT_USER\\{}\\{} registry entry: 0x{:08x}", path, name, status));
    default: hi_log_fatal("Error reading HKEY_CURRENT_USER\\{}\\{} registry entry: 0x{:08x}", path, name, status);
    }

    return static_cast<uint32_t>(result);
}

[[nodiscard]] std::vector<std::string> registry_read_current_user_multi_string(std::string_view path, std::string_view name)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);

    wchar_t initial_buffer[64];
    wchar_t *result = initial_buffer;
    DWORD result_length = sizeof(initial_buffer);

    for (auto repeat = 0; repeat != 5; ++repeat) {
        hilet status =
            RegGetValueW(HKEY_CURRENT_USER, wpath.c_str(), wname.c_str(), RRF_RT_REG_MULTI_SZ, NULL, result, &result_length);

        if (status == ERROR_SUCCESS) {
            auto r = ZZWSTR_to_string(result, result + result_length);
            if (result != initial_buffer) {
                delete[] result;
            }
            return r;
        }

        if (result != initial_buffer) {
            delete[] result;
        }

        switch (status) {
        case ERROR_MORE_DATA: result = new wchar_t[result_length]; break;

        case ERROR_BAD_PATHNAME:
        case ERROR_FILE_NOT_FOUND:
            throw os_error(std::format("Missing HKEY_CURRENT_USER\\{}\\{} registry entry: 0x{:08x}", path, name, status));
        default: hi_log_fatal("Error reading HKEY_CURRENT_USER\\{}\\{} registry entry: 0x{:08x}", path, name, status);
        }
    }

    throw os_error(std::format("Size requirements for HKEY_CURRENT_USER\\{}\\{} keeps changing", path, name));
}

} // namespace hi::inline v1
