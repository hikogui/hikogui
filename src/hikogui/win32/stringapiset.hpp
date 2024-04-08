// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "win32_error_intf.hpp"
#include <string>
#include <expected>
#include <vector>
#include <Windows.h>
#include <stringapiset.h>

hi_export_module(hikogui.win32 : stringapiset);

hi_export namespace hi { inline namespace v1 {

/** Convert a win32-API compatible std::wstring to a multi-byte std::string.
 * 
 * @param s The wide string to convert
 * @param code_page The code-page to use for conversion
 * @param flags The flags to passing
 * @return multi-byte string.
 */
[[nodiscard]] inline std::expected<std::string, win32_error> win32_WideCharToMultiByte(std::wstring_view s, unsigned int code_page = CP_UTF8, uint32_t flags = 0) noexcept
{
    if (s.empty()) {
        // WideCharToMultiByte() does not handle empty strings, if it can not also convert the null-character.
        return std::string{};
    }

    auto s_len = static_cast<int>(static_cast<unsigned int>(s.size()));
    auto r_len = ::WideCharToMultiByte(code_page, flags, s.data(), s_len, nullptr, 0, nullptr, nullptr);
    if (r_len == 0) {
        return std::unexpected{win32_GetLastError()};
    }

    auto r = std::string(static_cast<size_t>(static_cast<std::make_signed_t<size_t>>(r_len)), '\0');
    r.resize_and_overwrite(r_len, [&](char *p, size_t count) {
        return ::WideCharToMultiByte(code_page, flags, s.data(), s_len, p, static_cast<int>(count), nullptr, nullptr);
    });

    if (r.empty()) {
        return std::unexpected{win32_GetLastError()};
    }

    return r;
}

/** Convert a win32-API compatible std::wstring to a multi-byte std::string.
 * 
 * @param s The wide string to convert
 * @param code_page The code-page to use for conversion
 * @param flags The flags to passing
 * @return multi-byte string.
 */
[[nodiscard]] inline std::expected<std::wstring, win32_error> win32_MultiByteToWideChar(std::string_view s, unsigned int code_page = CP_UTF8, uint32_t flags = 0) noexcept
{
    if (s.empty()) {
        // MultiByteToWideChar() does not handle empty strings, if it can not also convert the null-character.
        return std::wstring{};
    }

    auto s_len = static_cast<int>(static_cast<unsigned int>(s.size()));
    auto r_len = ::MultiByteToWideChar(code_page, flags, s.data(), s_len, nullptr, 0);
    if (r_len == 0) {
        return std::unexpected{win32_GetLastError()};
    }

    auto r = std::wstring{};
    r.resize_and_overwrite(r_len, [&](wchar_t *p, size_t count) {
        return ::MultiByteToWideChar(code_page, flags, s.data(), s_len, p, static_cast<int>(count));
    });

    if (r.empty()) {
        return std::unexpected{win32_GetLastError()};
    }

    return r;
}

/** Convert a win32 zero terminated list of zero terminated strings.
 * 
 * This function will treat the array as-if it is a list of zero terminated strings,
 * where the last string is a zero terminated empty string.
 * 
 * @param first A pointer to a buffer of a zero terminated list of zero terminated string.
 * @param last A pointer one beyond the buffer.
 * @return A vector of UTF-8 encoded strings, win32_error::invalid_data when the list is incorrectly terminated.
 */
[[nodiscard]] inline std::expected<std::vector<std::string>, win32_error> win32_MultiSZToStringVector(wchar_t const *first, wchar_t const *last) noexcept
{
    auto r = std::vector<std::string>{};

    while (first != last) {
        auto it_zero = std::find(first, last, wchar_t{0});
        if (it_zero == last) {
            // No termination found.
            return std::unexpected{win32_error::invalid_data};
        }

        auto const ws = std::wstring_view{first, static_cast<std::size_t>(it_zero - first)};
        if (ws.empty()) {
            // The list is terminated with an empty string.
            break;
        }

        if (auto s = win32_WideCharToMultiByte(ws)) {
            r.push_back(*s);
        } else {
            return std::unexpected{s.error()};
        }

        // Continue after the zero terminator.
        first = it_zero + 1;
    }

    return r;
}

}}
