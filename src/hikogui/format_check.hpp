

#pragma once

#include <string_view>

namespace hi::inline v1 {


/** Count arguments of a std::format format string.
 *
 * @param fmt The format string.
 * @return The number of arguments required for formatting.
 * @retval -1 Invalid open-brace inside format argument.
 * @retval -2 Invalid close-brace outside format argument.
 * @retval -3 Missing close-brace at end-of-string.
 */
consteval int format_count(std::string_view fmt) noexcept
{
    auto tmp = std::vector<char>{};

    auto num_args = 0;
    auto o_count = 0;
    auto c_count = 0;
    auto is_open = false;
    auto prev = ' ';
    for (auto c: fmt) {
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

    if (o_count % 2) {
        return -3;

    } else if (c_count % 2) {
        if (not is_open) {
            return -2;
        }
        ++num_args;
    }

    return count;
}

template<typename... Args>
consteval int format_check(std::string_view fmt, Args const &... args) noexcept
{
    auto count = format_count(fmt);
    if (count < 0) {
        return count;
    } else if (count != sizeof...(args)) {
        return -4;
    }

}

#define hi_format_check(fmt, ...)\
    static_assert(::hi::format_count(fmt) != -1, "")

}

