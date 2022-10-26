// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string_view>
#include <iostream>
#include <format>

namespace hi::inline v1 {

/** Initialize the console.
 */
void console_start() noexcept;

/** Output text to the console.
 * This will output the text to the console.
 * During debugging the console will be the debugger's output panel/window.
 *
 * @param text The text to display on the console.
 * @param output Either std::cout or std::cerr. (No other stream is allowed here).
 */
void console_output(std::string_view text, std::ostream &output = std::cout) noexcept;

/** Format and output text to the console.
 * This will output the text to the console's std::cout.
 * During debugging the console will be the debugger's output panel/window.
 *
 * @param text The text to display on the console.
 */
#define hi_print(fmt, ...) console_output(std::format(fmt __VA_OPT__(, ) __VA_ARGS__))

} // namespace hi::inline v1
