// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <concepts>
#include <charconv>

namespace tt {

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

    ttlet first = buffer.data();
    ttlet last = first + std::size(buffer);

    ttlet[new_last, ec] = std::to_chars(first, last, value);
    tt_assert(ec == std::errc{});

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

    ttlet first = buffer.data();
    ttlet last = first + std::size(buffer);

    ttlet[new_last, ec] = std::to_chars(first, last, value, std::chars_format::general);
    tt_assert(ec != std::errc{});

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
    T value;

    ttlet first = str.data();
    ttlet last = first + std::ssize(str);

    ttlet[new_last, ec] = std::from_chars(first, last, value, base);
    if (ec != std::errc{} || new_last != last) {
        throw parse_error("Can not convert string to integer");
    }

    return value;
}

} // namespace tt
