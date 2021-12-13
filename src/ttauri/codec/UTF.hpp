// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../cast.hpp"
#include <string>
#include <string_view>

namespace tt::inline v1 {

/** Guess the endianness of an UTF-16 string.
 *
 * @param first The iterator pointing to an array of 8-bit char type.
 * @param last The one beyond the last position of the array.
 * @param default_guess The default endianness.
 * @return The endianness that was guessed based on the byte array.
 */
template<typename It>
[[nodiscard]] constexpr std::endian guess_utf16_endianess(It first, It last, std::endian default_guess)
{
    static_assert(sizeof(*first) == 1, "Expecting an array of 8-bit characters");
    ttlet num_words = narrow_cast<std::size_t>(std::distance(first, last) / 2);

    if (not num_words) {
        return default_guess;
    }

    // Check for BOM.
    {
        ttlet c0 = static_cast<uint8_t>(*first);
        ttlet c1 = static_cast<uint8_t>(*(first + 1));
        if (c0 == 0xfe && c1 == 0xff) {
            return std::endian::big;
        } else if (c1 == 0xfe and c0 == 0xff) {
            return std::endian::little;
        }
    }

    // Count the nul bytes in high or low byte of the UTF16 string.
    std::size_t count0 = 0;
    std::size_t count1 = 0;
    auto it = first;
    for (auto i = 0; i != num_words; ++i) {
        ttlet c0 = static_cast<uint8_t>(*(it++));
        ttlet c1 = static_cast<uint8_t>(*(it++));

        if (c0 == 0 and c0 != c1) {
            ++count0;
        } else if (c1 == 0 and c0 != c1) {
            ++count1;
        }
    }

    // Check for at least 1/8 ASCII characters.
    if (count0 == count1) {
        return default_guess;
    } else if (count0 > count1 and count0 > (num_words / 8)) {
        return std::endian::little;
    } else if (count1 > count0 and count1 > (num_words / 8)) {
        return std::endian::big;
    } else {
        return default_guess;
    }
}

/** Convert a UTF-8 string to a valid UTF-32 string.
 *
 * This conversion will replace invalid UTF-8 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Invalid code-unit
 *  - Continuation code-unit without start code-unit
 *  - Invalid number of continuation code-units after a start code-unit
 *  - Code-points in range of UTF-16 surrogate pair code-units.
 *  - Code-points beyond U+10ffff.
 *
 * @param rhs The UTF-8 string to decode
 * @return The resulting UTF-32 string.
 */
[[nodiscard]] std::u32string utf8_to_utf32(std::string_view rhs) noexcept;

/** Convert a UTF-8 string to a valid UTF-16 string.
 *
 * This conversion will replace invalid UTF-8 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Invalid code-unit
 *  - Continuation code-unit without start code-unit
 *  - Invalid number of continuation code-units after a start code-unit
 *  - Code-points in range of UTF-16 surrogate pair code-units.
 *  - Code-points beyond U+10ffff.
 *
 * @param rhs The UTF-8 string to decode
 * @return The resulting UTF-16 string.
 */
[[nodiscard]] std::u16string utf8_to_utf16(std::string_view rhs) noexcept;

/** Convert a UTF-8 string to a valid UTF-8 string.
 *
 * This conversion will replace invalid UTF-8 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Invalid code-unit
 *  - Continuation code-unit without start code-unit
 *  - Invalid number of continuation code-units after a start code-unit
 *  - Code-points in range of UTF-16 surrogate pair code-units.
 *  - Code-points beyond U+10ffff.
 *
 * @param rhs The UTF-8 string to decode
 * @return The resulting UTF-16 string.
 */
[[nodiscard]] std::string utf8_to_utf8(std::string_view rhs) noexcept;

/** Convert a UTF-8 string to a valid wide-string.
 *
 * This conversion will replace invalid UTF-8 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Invalid code-unit
 *  - Continuation code-unit without start code-unit
 *  - Invalid number of continuation code-units after a start code-unit
 *  - Code-points in range of UTF-16 surrogate pair code-units.
 *  - Code-points beyond U+10ffff.
 *
 * @param rhs The UTF-8 string to decode
 * @return The resulting wide string.
 */
[[nodiscard]] std::wstring utf8_to_wide(std::string_view rhs) noexcept;

/** Convert a UTF-16 string to a valid UTF-32 string.
 *
 * This conversion will replace invalid UTF-16 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Low-surrogate without a preceding high-surrogate.
 *  - Missing low-surrogate after a high-surrogate.
 *
 * @param rhs The UTF-16 string to decode
 * @return The resulting UTF-32 string.
 */
[[nodiscard]] std::u32string utf16_to_utf32(std::u16string_view rhs) noexcept;

/** Convert a UTF-16 string to a valid UTF-16 string.
 *
 * This conversion will replace invalid UTF-16 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Low-surrogate without a preceding high-surrogate.
 *  - Missing low-surrogate after a high-surrogate.
 *
 * @param rhs The UTF-16 string to decode
 * @return The resulting UTF-16 string.
 */
[[nodiscard]] std::u16string utf16_to_utf16(std::u16string_view rhs) noexcept;

/** Convert a UTF-16 string to a valid UTF-8 string.
 *
 * This conversion will replace invalid UTF-16 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Low-surrogate without a preceding high-surrogate.
 *  - Missing low-surrogate after a high-surrogate.
 *
 * @param rhs The UTF-16 string to decode
 * @return The resulting UTF-8 string.
 */
[[nodiscard]] std::string utf16_to_utf8(std::u16string_view rhs) noexcept;

/** Convert a UTF-16 string to a valid wide string.
 *
 * This conversion will replace invalid UTF-16 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Low-surrogate without a preceding high-surrogate.
 *  - Missing low-surrogate after a high-surrogate.
 *
 * @param rhs The UTF-16 string to decode
 * @return The resulting wide string.
 */
[[nodiscard]] std::wstring utf16_to_wide(std::u16string_view rhs) noexcept;

/** Convert a UTF-32 string to a valid UTF-32 string.
 *
 * This conversion will replace invalid UTF-32 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Code-points part of the UTF-16 surrogate pairs.
 *  - Code-points beyond U+10ffff.
 *
 * @param rhs The UTF-32 string to decode
 * @return The resulting UTF-32 string.
 */
[[nodiscard]] std::u32string utf32_to_utf32(std::u32string_view rhs) noexcept;

/** Convert a UTF-32 string to a valid UTF-16 string.
 *
 * This conversion will replace invalid UTF-32 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Code-points part of the UTF-16 surrogate pairs.
 *  - Code-points beyond U+10ffff.
 *
 * @param rhs The UTF-32 string to decode
 * @return The resulting UTF-16 string.
 */
[[nodiscard]] std::u16string utf32_to_utf16(std::u32string_view rhs) noexcept;

/** Convert a UTF-32 string to a valid UTF-8 string.
 *
 * This conversion will replace invalid UTF-32 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Code-points part of the UTF-16 surrogate pairs.
 *  - Code-points beyond U+10ffff.
 *
 * @param rhs The UTF-32 string to decode
 * @return The resulting UTF-8 string.
 */
[[nodiscard]] std::string utf32_to_utf8(std::u32string_view rhs) noexcept;

/** Convert a UTF-32 string to a valid wide string.
 *
 * This conversion will replace invalid UTF-32 sequences with the
 * unicode replacement character U+fffd. The following invalid sequences are detected:
 *  - Code-points part of the UTF-16 surrogate pairs.
 *  - Code-points beyond U+10ffff.
 *
 * @param rhs The UTF-32 string to decode
 * @return The resulting wide string.
 */
[[nodiscard]] std::wstring utf32_to_wide(std::u32string_view rhs) noexcept;

/** Convert a wide string to a valid UTF-32 string.
 *
 * This conversion will replace invalid UTF sequences with the
 * unicode replacement character U+fffd.
 *
 * @param rhs The wide string to decode
 * @return The resulting UTF-32 string.
 */
[[nodiscard]] std::u32string wide_to_utf32(std::wstring_view rhs) noexcept;

/** Convert a wide string to a valid UTF-16 string.
 *
 * This conversion will replace invalid UTF sequences with the
 * unicode replacement character U+fffd.
 *
 * @param rhs The wide string to decode
 * @return The resulting UTF-16 string.
 */
[[nodiscard]] std::u16string wide_to_utf16(std::wstring_view rhs) noexcept;

/** Convert a wide string to a valid UTF-8 string.
 *
 * This conversion will replace invalid UTF sequences with the
 * unicode replacement character U+fffd.
 *
 * @param rhs The wide string to decode
 * @return The resulting UTF-8 string.
 */
[[nodiscard]] std::string wide_to_utf8(std::wstring_view rhs) noexcept;

/** Convert a wide string to a valid wide string.
 *
 * This conversion will replace invalid UTF sequences with the
 * unicode replacement character U+fffd.
 *
 * @param rhs The wide string to decode
 * @return The resulting side string.
 */
[[nodiscard]] std::wstring wide_to_wide(std::wstring_view rhs) noexcept;

[[nodiscard]] std::string to_string(std::u16string_view rhs) noexcept;

[[nodiscard]] std::string to_string(std::u32string_view rhs) noexcept;

[[nodiscard]] std::string to_string(std::wstring_view rhs) noexcept;

[[nodiscard]] std::u16string to_u16string(std::string_view rhs) noexcept;

[[nodiscard]] std::u16string to_u16string(std::wstring_view rhs) noexcept;

[[nodiscard]] std::u16string to_u16string(std::u32string_view rhs) noexcept;

[[nodiscard]] std::u32string to_u32string(std::string_view rhs) noexcept;

[[nodiscard]] std::u32string to_u32string(std::u16string_view rhs) noexcept;

[[nodiscard]] std::u32string to_u32string(std::wstring_view rhs) noexcept;

[[nodiscard]] std::wstring to_wstring(std::string_view rhs) noexcept;

[[nodiscard]] std::wstring to_wstring(std::u16string_view rhs) noexcept;

[[nodiscard]] std::wstring to_wstring(std::u32string_view rhs) noexcept;

} // namespace tt::inline v1
