// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include <string_view>
#include <type_traits>

namespace hi::inline v1 {

/** Count arguments of a std::format format string.
 *
 * @param fmt The format string.
 * @return The number of arguments required for formatting.
 * @retval -1 Invalid open-brace inside format argument.
 * @retval -2 Invalid close-brace outside format argument.
 * @retval -3 Missing close-brace at end-of-string.
 */
constexpr int format_count(std::string_view fmt) noexcept
{
    auto num_args = 0;
    auto o_count = 0;
    auto c_count = 0;
    auto is_open = false;
    auto prev = ' ';
    for (auto c : fmt) {
        if (c != prev) {
            if (o_count % 2) {
                if (is_open) {
                    return -1;
                }
                is_open = true;

            } else if (c_count % 2) {
                if (not is_open) {
                    return -2;
                }
                ++num_args;
                is_open = false;
            }
        }

        o_count = c == '{' ? o_count + 1 : 0;
        c_count = c == '}' ? c_count + 1 : 0;
        prev = c;
    }

    if (c_count % 2) {
        if (not is_open) {
            return -2;
        }
        ++num_args;

    } else if (is_open) {
        return -3;
    }

    return num_args;
}

#define hi_format_argument_check(arg) \
    static_assert( \
        ::std::is_default_constructible_v<std::formatter<std::decay_t<decltype(arg)>>>, \
        "std::format, argument '" #arg "' does not have a specialized std::formatter<>.");

/** A macro to check if the format string and the arguments are valid for std::format.
 *
 * This macro checks if the usage of braces '{' and '}' are correctly used in the format-string.
 * Then it will check if the number of arguments match the number of arguments in the format-string.
 * Lastly it will check if the type for each argument has a valid `std::formatter<>` specialization.
 * 
 * This is done in a macro instead of a function, so that the static_asserts will point to the line
 * where the format-string and arguments where defined.
 * 
 * @param fmt The `std::format` format-string.
 * @param ... The arguments to be formatted by `std::format`.
 */
#define hi_format_check(fmt, ...) \
    static_assert(::hi::format_count(fmt) != -1, "std::format, Unexpected '{' inside argument-format."); \
    static_assert(::hi::format_count(fmt) != -2, "std::format, Unexpected '}' without corresponding '{'."); \
    static_assert(::hi::format_count(fmt) != -3, "std::format, Missing '}' at end of format string."); \
    static_assert( \
        ::hi::format_count(fmt) == hi_num_va_args(__VA_ARGS__), "std::format, invalid number of arguments for format string."); \
    hi_for_each(hi_format_argument_check, __VA_ARGS__)

} // namespace hi::inline v1
