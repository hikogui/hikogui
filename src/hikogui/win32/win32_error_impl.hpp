// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "win32_error_intf.hpp"

hi_export_module(hikogui.win32 : win32_error_impl);

hi_export namespace hi { inline namespace v1 {

[[nodiscard]] inline std::string win32_error_category::message(int code) const
{
    if (auto msg = win32_FormatMessage(static_cast<win32_error>(code))) {
        return *msg;

    } else {
        throw std::system_error(msg.error());
    }
}

}}
