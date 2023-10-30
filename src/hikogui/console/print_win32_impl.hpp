// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"

#include "print_intf.hpp"
#include "../utility/utility.hpp"
#include "../char_maps/char_maps.hpp"
#include "../macros.hpp"
#include <memory>
#include <string_view>
#include <format>
#include <iostream>

hi_export_module(hikogui.console.print : impl);

hi_export namespace hi::inline v1 {


hi_export template<typename... Args>
inline void print(std::format_string<Args...> fmt, Args&&... args) noexcept
{
    auto msg = std::format(fmt, std::forward<Args>(args)...);

    if (IsDebuggerPresent()) {
        hilet text_ = to_wstring(msg);
        OutputDebugStringW(text_.c_str());

    } else {
        std::cout << msg;
    }
}

hi_export template<typename... Args>
inline void println(std::format_string<Args...> fmt, Args&&... args) noexcept
{
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    msg += '\n';

    if (IsDebuggerPresent()) {
        hilet text_ = to_wstring(msg);
        OutputDebugStringW(text_.c_str());

    } else {
        std::cout << msg;
    }
}


} // namespace hi::inline v1
