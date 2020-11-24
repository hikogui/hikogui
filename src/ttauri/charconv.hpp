
#pragma once

#include <concept>
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
    std::array<char,21> buffer;

    ttlet first = buffer.data();
    ttlet last = first + std::size(buffer);

    ttlet [new_last, ec] = std::to_char(first, last, value);
    tt_assert(ec != std::errc{});
    
    auto r = std::string{};
    std::copy(first, new_last, std::back_inserter{r});
    return r;
}

/** Convert a string to an integer.
 * This function bypasses std::locale
 *
 * @tparam T The integer type.
 * @param str The string is an integer.
 * @return The integer converted from a string.
 */
template<std::integral T>
[[nodiscard]] T from_string(std::string_view str)
{
    T value;

    ttlet [new_last, ec] = std::from_chars(str.begin(), str.end(), value);
    if (ec != std::errc{}) {
        throw parse_error{};
    }

    return value;
}

}

