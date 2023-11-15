// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "../macros.hpp"
#include "base.hpp"
#include <string_view>
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <cassert>

hi_export_module(hikogui.win32.winreg);

hi_export namespace hi { inline namespace v1 {

/** Delete a registry value.
 *
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 */
hi_inline win32_error win32_RegDeleteKeyValue(HKEY key, std::string_view path, std::string_view name) noexcept
{
    hilet wpath = win32_MultiByteToWideChar(path);
    if (not wpath) {
        return wpath.error();
    }

    hilet wname = win32_MultiByteToWideChar(name);
    if (not wname) {
        return wname.error();
    }

    return static_cast<win32_error>(::RegDeleteKeyValueW(key, wpath->c_str(), wname->c_str()));
}

/** Delete all registry values and the last part of the subkey.
 *
 * @param key The registry's key
 * @param path The path to the values.
 */
hi_inline win32_error win32_RegDeleteKey(HKEY key, std::string_view path) noexcept
{
    hilet wpath = win32_MultiByteToWideChar(path);
    if (not wpath) {
        return wpath.error();
    }

    return static_cast<win32_error>(::RegDeleteKeyW(key, wpath->c_str()));
}

/** Write a DWORD registry value.
 *
 * @note If the path or name do not exist it is automatically created.
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @param value The value to write
 * @return 0 on success, or error code on failure.
 */
[[nodiscard]] hi_inline win32_error win32_RegSetKeyValue(HKEY key, std::string_view path, std::string_view name, uint32_t value)
{
    hilet wpath = win32_MultiByteToWideChar(path);
    if (not wpath) {
        return wpath.error();
    }

    hilet wname = win32_MultiByteToWideChar(name);
    if (not wname) {
        return wname.error();
    }

    hilet dvalue = static_cast<DWORD>(value);

    return static_cast<win32_error>(::RegSetKeyValueW(key, wpath->c_str(), wname->c_str(), REG_DWORD, &dvalue, sizeof(dvalue)));
}

/** Write a string registry value.
 *
 * @note If the path or name do not exist it is automatically created.
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @param value The value to write
 * @return 0 on success, or error code on failure.
 */
[[nodiscard]] hi_inline win32_error win32_RegSetKeyValue(HKEY key, std::string_view path, std::string_view name, std::string_view value)
{
    hilet wpath = win32_MultiByteToWideChar(path);
    if (not wpath) {
        return wpath.error();
    }

    hilet wname = win32_MultiByteToWideChar(name);
    if (not wname) {
        return wname.error();
    }

    hilet wvalue = win32_MultiByteToWideChar(value);
    if (not wvalue) {
        return wvalue.error();
    }

    hilet wvalue_size = static_cast<DWORD>((wvalue->size() + 1) * sizeof(wchar_t));

    return static_cast<win32_error>(::RegSetKeyValueW(key, wpath->c_str(), wname->c_str(), REG_SZ, wvalue->c_str(), wvalue_size));
}

/** Check if a registry entry exists.
 * @return win32_error success, or win32_error::file_not_found if entry was not found, otherwise an error.
 */
[[nodiscard]] hi_inline std::expected<void, win32_error> win32_RegGetValue_void(HKEY key, std::string_view path, std::string_view name)
{
    hilet wpath = win32_MultiByteToWideChar(path);
    if (not wpath) {
        return std::unexpected{wpath.error()};
    }

    hilet wname = win32_MultiByteToWideChar(name);
    if (not wname) {
        return std::unexpected{wname.error()};
    }

    auto status = static_cast<win32_error>(::RegGetValueW(key, wpath->c_str(), wname->c_str(), RRF_RT_ANY, NULL, NULL, NULL));
    if (status == win32_error::success) {
        return {};
    } else {
        return std::unexpected{status};
    }
}

/** Read a DWORD registry value.
 *
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @return value, or win32_error::file_not_found if entry was not found, otherwise an error.
 */
[[nodiscard]] hi_inline std::expected<uint32_t, win32_error> win32_RegGetValue_dword(HKEY key, std::string_view path, std::string_view name) noexcept
{
    hilet wpath = win32_MultiByteToWideChar(path);
    if (not wpath) {
        return std::unexpected{wpath.error()};
    }

    hilet wname = win32_MultiByteToWideChar(name);
    if (not wname) {
        return std::unexpected{wname.error()};
    }

    DWORD result;
    DWORD result_length = sizeof(result);
    DWORD result_type = 0;
    hilet status = static_cast<win32_error>(::RegGetValueW(key, wpath->c_str(), wname->c_str(), RRF_RT_ANY, &result_type, &result, &result_length));

    if (static_cast<bool>(status)) {
        return std::unexpected{status};
    }
   
    return static_cast<uint32_t>(result);
}

/** Read a strings from the registry value.
 *
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @return value, or win32_error::file_not_found if entry was not found, otherwise an error.
 */
[[nodiscard]] hi_inline std::expected<std::string, win32_error> win32_RegGetValue_string(HKEY key, std::string_view path, std::string_view name) noexcept
{
    hilet wpath = win32_MultiByteToWideChar(path);
    if (not wpath) {
        return std::unexpected{wpath.error()};
    }

    hilet wname = win32_MultiByteToWideChar(name);
    if (not wname) {
        return std::unexpected{wname.error()};
    }

    auto result = std::wstring{};

    auto expected_size = size_t{64};
    for (auto repeat = 0; repeat != 5; ++repeat) {
        auto status = win32_error{};

        result.resize_and_overwrite(expected_size, [&](auto p, auto n) {
            // size includes the null-terminator.
            auto result_length = static_cast<DWORD>((n + 1) * sizeof(wchar_t));
            status = static_cast<win32_error>(::RegGetValueW(key, wpath->c_str(), wname->c_str(), RRF_RT_REG_SZ, NULL, p, &result_length));

            expected_size = (result_length / sizeof(wchar_t)) - 1;
            return std::min(n, expected_size);
        });

        switch (status) {
        case win32_error::success:
            return win32_WideCharToMultiByte(result);
        case win32_error::more_data:
            continue;
        default:
            return std::unexpected{status};
        }
    }

    // Data size keeps changing.
    return std::unexpected{win32_error::more_data};
}

/** Read a list of strings from the registry value.
 *
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @return value, or std::nullopt if the registry-value was not found.
 * @throws hi::os_error Unable to read the registry-value, for example when the type was different.
 */
[[nodiscard]] hi_inline std::expected<std::vector<std::string>, win32_error>
win32_RegGetValue_multi_string(HKEY key, std::string_view path, std::string_view name) noexcept
{
    hilet wpath = win32_MultiByteToWideChar(path);
    if (not wpath) {
        return std::unexpected{wpath.error()};
    }

    hilet wname = win32_MultiByteToWideChar(name);
    if (not wname) {
        return std::unexpected{wname.error()};
    }

    auto result = std::wstring{};
    result.resize(64);

    for (auto repeat = 0; repeat != 5; ++repeat) {
        auto result_length = static_cast<DWORD>(result.size() * sizeof(wchar_t));
        hilet status =
            static_cast<win32_error>(::RegGetValueW(key, wpath->c_str(), wname->c_str(), RRF_RT_REG_MULTI_SZ, NULL, result.data(), &result_length));

        switch (status) {
        case win32_error::success:
            return win32_MultiSZToStringVector(result.data(), result.data() + result_length);

        case win32_error::more_data:
            assert(result_length % 2 == 0);
            result.resize(result_length / sizeof(wchar_t));
            break;

        default:
            return std::unexpected{status};
        }
    }

    // Data size keeps changing.
    return std::unexpected{win32_error::more_data};
}

/** Read from the registry value.
 *
 * @tparam T The type of the value to read.
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @return value, or std::nullopt if the registry-value was not found.
 * @throws hi::os_error Unable to read the registry-value, for example when the type was different.
 */
template<typename T>
[[nodiscard]] std::expected<T, win32_error> win32_RegGetValue(HKEY key, std::string_view path, std::string_view name) = delete;

template<>
[[nodiscard]] hi_inline std::expected<void, win32_error> win32_RegGetValue(HKEY key, std::string_view path, std::string_view name)
{
    return win32_RegGetValue_void(key, path, name);
}

template<std::integral T>
[[nodiscard]] hi_inline std::expected<T, win32_error> win32_RegGetValue(HKEY key, std::string_view path, std::string_view name)
{
    if (hilet tmp = win32_RegGetValue_dword(key, path, name)) {
        return static_cast<T>(*tmp);
    } else {
        return std::unexpected{tmp.error()};
    }
}

template<>
[[nodiscard]] hi_inline std::expected<std::string, win32_error> win32_RegGetValue(HKEY key, std::string_view path, std::string_view name)
{
    return win32_RegGetValue_string(key, path, name);
}

template<>
[[nodiscard]] hi_inline std::expected<std::vector<std::string>, win32_error>
win32_RegGetValue(HKEY key, std::string_view path, std::string_view name)
{
    return win32_RegGetValue_multi_string(key, path, name);
}

}} // namespace hi::inline v1
