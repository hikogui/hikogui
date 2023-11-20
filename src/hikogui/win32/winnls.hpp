// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "base.hpp"
#include <expected>
#include <string>
#include <system_error>

hi_export_module(hikogui.win32.winnls);

hi_export namespace hi { inline namespace v1 {

[[nodiscard]] hi_inline std::expected<std::string, win32_error> win32_GetUserDefaultLocaleName() noexcept
{
    auto code = win32_error{};
    
    auto name = std::wstring{};
    name.resize_and_overwrite(LOCALE_NAME_MAX_LENGTH - 1, [&code](wchar_t *p, size_t count) {
        if (auto actual_count = ::GetUserDefaultLocaleName(p, static_cast<int>(count + 1))) {
            return actual_count - 1;
        } else {
            code = win32_GetLastError();
            return 0;
        }
    });

    if (static_cast<bool>(code)) {
        return std::unexpected{code};
    }

    return win32_WideCharToMultiByte(name);
}

}}
