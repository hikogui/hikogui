// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "win32_error_intf.hpp"
#include "stringapiset.hpp"
#include <string>
#include <expected>
#include <Windows.h>
#include <WinBase.h>

hi_export_module(hikogui.win32 : winbase);

hi_export namespace hi { inline namespace v1 {

[[nodiscard]] hi_inline std::expected<std::string, win32_error> win32_FormatMessage(win32_error error_code) noexcept
{
    auto const error_code_ = static_cast<DWORD>(std::to_underlying(error_code));

    // FormatMessageW() is unable to tell what the buffer size should be.
    // But 64Kbyte is the largest buffer that one should pass.
    LPWSTR buffer = nullptr;
    auto const result = ::FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // source
        error_code_,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer),
        0,
        NULL);

    if (result == 0) {
        return std::unexpected(win32_GetLastError());
    }

    auto r = win32_WideCharToMultiByte(std::wstring_view{buffer, result});
    LocalFree(buffer);
    return r;
}

}}
