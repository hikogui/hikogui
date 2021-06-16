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


///** Convert a text to an integer.
// */
//template<std::integral T>
//[[nodiscard]] T from_string(std::locale const &loc, std::string const &str, int base = 10)
//{
//    if constexpr (std::is_same_v<T, int>) {
//        return std::stoi(str, pos, base);
//    } else if constexpr (std::is_same_v<T, long>) {
//        return std::stol(str, pos, base);
//    } else if constexpr (std::is_same_v<T, long long>) {
//        return std::stoll(str, pos, base);
//    } else if constexpr (std::is_same_v<T, unsigned long>) {
//        return std::stoul(str, pos, base);
//    } else if constexpr (std::is_same_v<T, unsigned long long>) {
//        return std::stoull(str, pos, base);
//    } else {
//        tt_static_not_implemented();
//    }
//}
//
///** Convert a text to an integer.
// */
//template<std::integral T>
//[[nodiscard]] T from_string(std::locale const &locstd::string_view const &str, size_t *pos = nullptr, int base = 10)
//{
//    return to_integral<T>(loc, std::string{str}, pos, base);
//}

} // namespace tt
