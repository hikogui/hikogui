// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include <concepts>
#include <charconv>

namespace hi { inline namespace v1 {

/** Convert integer to string.
 * This function bypasses std::locale.
 *
 * @param value The signed or unsigned integer value.
 * @return The integer converted to a decimal string.
 */
template<std::integral T>
[[nodiscard]] std::string to_string(T const &value) noexcept
{
    std::array<char, 21> buffer;

    hilet first = buffer.data();
    hilet last = first + buffer.size();

    hilet[new_last, ec] = std::to_chars(first, last, value);
    hi_assert(ec == std::errc{});

    auto r = std::string{};
    std::copy(first, new_last, std::back_inserter(r));
    return r;
}

/** Convert floating point to string.
 * This function bypasses std::locale.
 *
 * @param value The signed or unsigned integer value.
 * @return The integer converted to a decimal string.
 */
template<std::floating_point T>
[[nodiscard]] std::string to_string(T const &value) noexcept
{
    std::array<char, 128> buffer;

    hilet first = buffer.data();
    hilet last = first + buffer.size();

    hilet[new_last, ec] = std::to_chars(first, last, value, std::chars_format::general);
    hi_assert(ec == std::errc{});

    auto r = std::string{};
    std::copy(first, new_last, std::back_inserter(r));
    return r;
}

/** Convert a string to an integer.
 * This function bypasses std::locale
 *
 * @tparam T The integer type.
 * @param str The string is an integer.
 * @param base The base radix of the string encoded integer.
 * @return The integer converted from a string.
 */
template<std::integral T>
[[nodiscard]] T from_string(std::string_view str, int base = 10)
{
    auto value = T{};

    hilet first = str.data();
    hilet last = first + ssize(str);

    hilet[new_last, ec] = std::from_chars(first, last, value, base);
    if (ec != std::errc{} or new_last != last) {
        throw parse_error("Can not convert string to integer");
    }

    return value;
}

/** Convert a string to an floating point.
 * This function bypasses std::locale
 *
 * @tparam T The integer type.
 * @param str The string is an integer.
 * @return The integer converted from a string.
 */
template<std::floating_point T>
[[nodiscard]] T from_string(std::string_view str)
{
    T value;

    hilet first = str.data();
    hilet last = first + ssize(str);

    hilet[new_last, ec] = std::from_chars(first, last, value);
    if (ec != std::errc{} or new_last != last) {
        throw parse_error("Can not convert string to floating point");
    }

    return value;
}

}} // namespace hi::inline v1
