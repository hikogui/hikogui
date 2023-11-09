// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "../macros.hpp"
#include "../win32/win32.hpp"
#include "defer.hpp"
#include "exception_intf.hpp"
#include <type_traits>
#include <string>
#include <string_view>
#include <format>

hi_export_module(hikogui.utility.exception : impl);

hi_export namespace hi { inline namespace v1 {

hi_export [[nodiscard]] hi_inline std::string get_last_error_message(uint32_t error_code)
{
    if (auto msg = win32_FormatMessage(static_cast<win32_error>(error_code))) {
        return *msg;
    } else {
        throw std::system_error(msg.error());
    }
}

hi_export [[nodiscard]] hi_inline std::string get_last_error_message()
{
    if (auto msg = win32_FormatMessage(win32_GetLastError())) {
        return *msg;
    } else {
        throw std::system_error(msg.error());
    }
}

}} // namespace hi::v1
