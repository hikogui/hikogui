// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../utility/win32_headers.hpp"
#include "registry_win32.hpp"
#include "../utility/module.hpp"
#include "../telemetry/module.hpp"
#include "../strings.hpp"

namespace hi::inline v1 {

[[nodiscard]] constexpr HKEY to_hkey(registry_key key) noexcept
{
    switch (key) {
    case registry_key::classes_root:
        return HKEY_CLASSES_ROOT;
    case registry_key::current_config:
        return HKEY_CURRENT_CONFIG;
    case registry_key::current_user:
        return HKEY_CURRENT_USER;
    case registry_key::local_machine:
        return HKEY_LOCAL_MACHINE;
    case registry_key::users:
        return HKEY_USERS;
    default:
        hi_no_default();
    }
}

[[nodiscard]] constexpr std::string to_string(registry_key key) noexcept
{
    switch (key) {
    case registry_key::classes_root:
        return "HKEY_CLASSES_ROOT";
    case registry_key::current_config:
        return "HKEY_CURRENT_CONFIG";
    case registry_key::current_user:
        return "HKEY_CURRENT_USER";
    case registry_key::local_machine:
        return "HKEY_LOCAL_MACHINE";
    case registry_key::users:
        return "HKEY_USERS";
    default:
        hi_no_default();
    }
}

bool registry_delete(registry_key key, std::string_view path, std::string_view name)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);

    hilet status = RegDeleteKeyValueW(to_hkey(key), wpath.c_str(), wname.c_str());
    switch (status) {
    case ERROR_SUCCESS:
        return true;
    case ERROR_FILE_NOT_FOUND:
        return false;
    default:
        throw os_error(std::format(
            "Error deleting {}\\{}\\{} registry entry: {}", to_string(key), path, name, get_last_error_message(status)));
    }
}

bool registry_delete(registry_key key, std::string_view path)
{
    hilet wpath = hi::to_wstring(path);

    hilet status = RegDeleteKeyW(to_hkey(key), wpath.c_str());
    switch (status) {
    case ERROR_SUCCESS:
        return true;
    case ERROR_FILE_NOT_FOUND:
        return false;
    default:
        throw os_error(std::format(
            "Error deleting {}\\{}\\ registry entry: {}", to_string(key), path, get_last_error_message(status)));
    }
}

[[nodiscard]] bool registry_exists(registry_key key, std::string_view path, std::string_view name)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);

    hilet status = RegGetValueW(to_hkey(key), wpath.c_str(), wname.c_str(), RRF_RT_ANY, NULL, NULL, NULL);
    switch (status) {
    case ERROR_SUCCESS:
        return true;
    case ERROR_FILE_NOT_FOUND:
        return false;
    default:
        throw os_error(std::format(
            "Error checking existence of {}\\{}\\{} registry entry: {}",
            to_string(key),
            path,
            name,
            get_last_error_message(status)));
    }
}

[[nodiscard]] void registry_write(registry_key key, std::string_view path, std::string_view name, uint32_t value)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);
    hilet dvalue = narrow_cast<DWORD>(value);

    if (hilet status = RegSetKeyValueW(to_hkey(key), wpath.c_str(), wname.c_str(), RRF_RT_DWORD, &dvalue, sizeof(dvalue));
        status != ERROR_SUCCESS) {
        throw os_error(std::format(
            "Error deleting {}\\{}\\{} = {} registry entry: {}",
            to_string(key),
            path,
            name,
            value,
            get_last_error_message(status)));
    }
}

[[nodiscard]] void registry_write(registry_key key, std::string_view path, std::string_view name, std::string_view value)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);
    hilet wvalue = hi::to_wstring(value);
    hilet wvalue_size = narrow_cast<DWORD>((wvalue.size() + 1) * sizeof(wchar_t));

    if (hilet status = RegSetKeyValueW(to_hkey(key), wpath.c_str(), wname.c_str(), RRF_RT_REG_SZ, wvalue.c_str(), wvalue_size);
        status != ERROR_SUCCESS) {
        throw os_error(std::format(
            "Error writing {}\\{}\\{} = \"{}\" registry entry: {}",
            to_string(key),
            path,
            name,
            value,
            get_last_error_message(status)));
    }
}

[[nodiscard]] std::optional<uint32_t> registry_read_dword(registry_key key, std::string_view path, std::string_view name)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);

    DWORD result;
    DWORD result_length = sizeof(result);
    DWORD result_type = 0;
    hilet status = RegGetValueW(to_hkey(key), wpath.c_str(), wname.c_str(), RRF_RT_ANY, &result_type, &result, &result_length);

    switch (status) {
    case ERROR_SUCCESS:
        break;

    case ERROR_FILE_NOT_FOUND:
        return std::nullopt;

    default:
        throw os_error(std::format(
            "Error reading {}\\{}\\{} registry entry: {}", to_string(key), path, name, get_last_error_message(status)));
    }

    if (result_type != RRF_RT_DWORD) {
        throw os_error(std::format("Incorrect type of {}\\{}\\{} registry entry.", to_string(key), path, name));
    }

    return static_cast<uint32_t>(result);
}

[[nodiscard]] std::optional<std::string> registry_read_string(registry_key key, std::string_view path, std::string_view name)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);

    auto result = std::wstring{};

    auto expected_size = 64_uz;
    for (auto repeat = 0; repeat != 5; ++repeat) {
        auto status = LSTATUS{};

        result.resize_and_overwrite(expected_size, [&](auto p, auto n) {
            // size includes the null-terminator.
            auto result_length = narrow_cast<DWORD>((n + 1) * sizeof(wchar_t));
            status = RegGetValueW(to_hkey(key), wpath.c_str(), wname.c_str(), RRF_RT_REG_SZ, NULL, p, &result_length);

            expected_size = (result_length / sizeof(wchar_t)) - 1;
            return std::min(n, expected_size);
        });

        switch (status) {
        case ERROR_SUCCESS:
            return to_string(result);

        case ERROR_MORE_DATA:
            continue;

        case ERROR_FILE_NOT_FOUND:
            return std::nullopt;

        default:
            throw os_error(std::format(
                "Error reading {}\\{}\\{} registry entry: {}", to_string(key), path, name, get_last_error_message(status)));
        }
    }

    throw os_error(std::format("Size requirements for {}\\{}\\{} keeps changing", to_string(key), path, name));
}

[[nodiscard]] std::optional<std::vector<std::string>>
registry_read_multi_string(registry_key key, std::string_view path, std::string_view name)
{
    hilet wpath = hi::to_wstring(path);
    hilet wname = hi::to_wstring(name);

    auto result = std::wstring{};
    result.resize(64);

    for (auto repeat = 0; repeat != 5; ++repeat) {
        auto result_length = narrow_cast<DWORD>(result.size() * sizeof(wchar_t));
        hilet status =
            RegGetValueW(to_hkey(key), wpath.c_str(), wname.c_str(), RRF_RT_REG_MULTI_SZ, NULL, result.data(), &result_length);

        switch (status) {
        case ERROR_SUCCESS:
            return ZZWSTR_to_string(result.data(), result.data() + result_length);

        case ERROR_MORE_DATA:
            hi_assert(result_length % 2 == 0);
            result.resize(result_length / sizeof(wchar_t));
            break;

        case ERROR_FILE_NOT_FOUND:
            return std::nullopt;

        default:
            throw os_error(std::format(
                "Error reading {}\\{}\\{} registry entry: {}", to_string(key), path, name, get_last_error_message(status)));
        }
    }

    throw os_error(std::format("Size requirements for {}\\{}\\{} keeps changing", to_string(key), path, name));
}

} // namespace hi::inline v1
