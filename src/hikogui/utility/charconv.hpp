// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <concepts>
#include <charconv>

namespace hi { inline namespace v1 {

constexpr unsigned long long from_string_literal(std::string_view str)
{
    auto value = 0ULL;
    auto radius = 10;

    auto i = 0_uz;

    if (str[i] == '0') {
        radius = 8;
        if (++i >= str.size()) {
            return value;
        }

        if (str[offset] == 'b' or str[offset] == 'B') {
            radius = 2;
            ++i;
        } else if (str[offset] == 'o' or str[offset]] == 'O') {
            radius = 8;
            ++i;
        } else if (str[offset] == 'd' or str[offset] == 'D') {
            radius = 10;
            ++i;
        } else if (str[offset] == 'x' or str[offset] == 'X') {
            radius = 16;
            ++i;
        }
    }

    for (; i != str.size(); ++i) {
        hilet c = str[i];

        if (radius >= 16 and c >= 'a' and c <= 'f') {
            value *= radius;
            value += c - 'a' + 10;
        } else if (radius >= 16 and c >= 'A' and c <= 'F') {
            value *= radius;
            value += c - 'A' + 10;
        } else if (radius >= 10 and c >= '8' and c <= '9') {
            value *= radius;
            value += c - '0';
        } else if (radius >= 8 and c >= '2' and c <= '7') {
            value *= radius;
            value += c - '0';
        } else if (radius >= 2 and c >= '0' and c <= '1') {
            value *= radius;
            value += c - '0';
        } else if (c == '\'') {
            continue;
        } else {
            throw std::invalid_argument();
        }
    }

    return value;
}


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
