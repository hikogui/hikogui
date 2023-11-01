// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"
#include "../win32_headers.hpp"


#include <memory>
#include <string_view>
#include <format>
#include <iostream>

export module hikogui_console_print : impl;
import : intf;
import hikogui_char_maps;
import hikogui_utility;

export namespace hi::inline v1 {


export template<typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args) noexcept
{
    auto msg = std::format(fmt, std::forward<Args>(args)...);

    if (IsDebuggerPresent()) {
        hilet text_ = to_wstring(msg);
        OutputDebugStringW(text_.c_str());

    } else {
        std::cout << msg;
    }
}

export template<typename... Args>
void println(std::format_string<Args...> fmt, Args&&... args) noexcept
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
