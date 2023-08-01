// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "../macros.hpp"
#include "defer.hpp"
#include "exception_intf.hpp"
#include <type_traits>
#include <string>
#include <string_view>
#include <format>

hi_export_module(hikogui.utility.exception : impl);

hi_export namespace hi {
inline namespace v1 {

/** Convert a win32-API compatible std::wstring to a UTF-8 std::string.
 */
[[nodiscard]] inline std::string win32_wstring_to_string(std::wstring_view s)
{
    auto s_len = static_cast<int>(static_cast<unsigned int>(s.size()));
    auto r_len = WideCharToMultiByte(CP_UTF8, 0, s.data(), s_len, nullptr, 0, nullptr, nullptr);
    if (r_len == 0) {
        throw parse_error("win32_wstring_to_string()");
    }

    auto r = std::string(static_cast<size_t>(static_cast<std::make_signed_t<size_t>>(r_len)), '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), s_len, r.data(), r_len, nullptr, nullptr);
    return r;
}

[[nodiscard]] inline std::string get_last_error_message(uint32_t error_code)
{
    hilet error_code_ = static_cast<DWORD>(error_code);

    // FormatMessageW() is unable to tell what the buffer size should be.
    // But 64Kbyte is the largest buffer that one should pass.
    LPWSTR buffer = nullptr;
    hilet result = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // source
        error_code_,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer),
        0,
        NULL);

    if (result == 0) {
        throw os_error(std::format("Could not format OS error message for error {}.", error_code));
    }

    hilet d = defer([&]{
        LocalFree(buffer);
    });

    return win32_wstring_to_string(std::wstring_view{buffer, result});
}

[[nodiscard]] inline uint32_t get_last_error_code() noexcept
{
    return static_cast<uint32_t>(GetLastError());
}

[[nodiscard]] inline std::string get_last_error_message() {
    return get_last_error_message(get_last_error_code());
}

}}
