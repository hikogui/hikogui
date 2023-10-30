// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <string_view>
#include <type_traits>

export module hikogui_telemetry_format_check;
import hikogui_utility;


export namespace hi::inline v1 {

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

} // namespace hi::inline v1
