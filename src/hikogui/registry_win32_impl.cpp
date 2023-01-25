// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "registry_win32.hpp"
#include "utility/module.hpp"
#include "strings.hpp"
#include "log.hpp"
#include "utility/win32_headers.hpp"

namespace hi::inline v1 {

[[nodiscard]] uint32_t registry_read_current_user_dword(std::string_view path, std::string_view name)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);

    DWORD result;
    DWORD result_length = sizeof(result);
    hilet status = RegGetValueW(HKEY_CURRENT_USER, wpath.c_str(), wname.c_str(), RRF_RT_DWORD, NULL, &result, &result_length);

    switch (status) {
    case ERROR_SUCCESS:
        break;
    case ERROR_BAD_PATHNAME:
    case ERROR_FILE_NOT_FOUND:
        throw os_error(std::format("Missing HKEY_CURRENT_USER\\{}\\{} registry entry: 0x{:08x}", path, name, status));
    default:
        hi_log_fatal("Error reading HKEY_CURRENT_USER\\{}\\{} registry entry: 0x{:08x}", path, name, status);
    }

    return static_cast<uint32_t>(result);
}

[[nodiscard]] std::vector<std::string> registry_read_current_user_multi_string(std::string_view path, std::string_view name)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);

    auto result = std::wstring{};
    result.resize(64);

    for (auto repeat = 0; repeat != 5; ++repeat) {
        auto result_length = narrow_cast<DWORD>(result.size() * sizeof(wchar_t));
        hilet status = RegGetValueW(
            HKEY_CURRENT_USER, wpath.c_str(), wname.c_str(), RRF_RT_REG_MULTI_SZ, NULL, result.data(), &result_length);

        switch (status) {
        case ERROR_SUCCESS:
            return ZZWSTR_to_string(result.data(), result.data() + result_length);

        case ERROR_MORE_DATA:
            hi_assert(result_length % 2 == 0);
            result.resize(result_length / sizeof(wchar_t));
            break;

        case ERROR_BAD_PATHNAME:
        case ERROR_FILE_NOT_FOUND:
            throw os_error(std::format("Missing HKEY_CURRENT_USER\\{}\\{} registry entry: 0x{:08x}", path, name, status));

        default:
            hi_log_fatal("Error reading HKEY_CURRENT_USER\\{}\\{} registry entry: 0x{:08x}", path, name, status);
        }
    }

    throw os_error(std::format("Size requirements for HKEY_CURRENT_USER\\{}\\{} keeps changing", path, name));
}

} // namespace hi::inline v1
