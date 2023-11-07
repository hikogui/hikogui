// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <string_view>
#include <iostream>
#include <format>

export module hikogui_console_print : intf;

export namespace hi { inline namespace v1 {

/** Output text to the console.
 * This will output the text to the console.
 * During debugging the console will be the debugger's output panel/window.
 *
 * @param text The text to display on the console.
 * @param output Either std::cout or std::cerr. (No other stream is allowed here).
 */
export template<typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args) noexcept;

/** Output a line of text to the console.
 * This will output the text to the console.
 * During debugging the console will be the debugger's output panel/window.
 *
 * @param text The text to display on the console.
 * @param output Either std::cout or std::cerr. (No other stream is allowed here).
 */
export template<typename... Args>
void println(std::format_string<Args...> fmt, Args&&... args) noexcept;


}} // namespace hi::inline v1

