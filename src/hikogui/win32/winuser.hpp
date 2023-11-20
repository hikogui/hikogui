// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "../macros.hpp"
#include "base.hpp"
#include <expected>
#include <string>
#include <string_view>
#include <system_error>

hi_export_module(hikogui.win32.winuser);

namespace hi { inline namespace v1 {

hi_inline std::expected<UINT, win32_error> win32_MessageBox(HWND handle, std::string_view text, std::string_view caption, UINT type) noexcept
{
    hilet wtext = win32_MultiByteToWideChar(text);
    if (not wtext) {
        return std::unexpected{wtext.error()};
    }

    hilet wcaption = win32_MultiByteToWideChar(caption);
    if (not wcaption) {
        return std::unexpected{wcaption.error()};
    }

    hilet r = MessageBoxW(handle, wtext->c_str(), wcaption->c_str(), type);
    if (r == 0) {
        return std::unexpected{win32_GetLastError()};
    }

    return r;
}

}}
