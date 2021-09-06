// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <string_view>
#include <bit>

namespace tt {
namespace detail {

[[nodiscard]] constexpr size_t num_code_points(std::string_view rhs) noexcept
{
    auto r = 0_uz;

    for (ttlet c : rhs) {
        if ((c & 0b1100'0000) != 0b1000'0000) {
            ++r;
        }
    }

    return r;
}

[[nodiscard]] constexpr size_t num_utf8_code_units(std::u32string_view rhs) noexcept
{
    auto r = 0_uz;

    for (ttlet c : rhs) {
        if (c < 0x80) {
            ++r;
        } else if (c < 0x800) {
            r += 2;
        } else if (c < 0x1'0000) {
            r += 3;
        } else {
            r += 4;
        }
    }

    return r;
}

[[nodiscard]] constexpr uint32_t utf8_start_value(uint8_t c) noexcept
{
    ttlet num = std:lcount_one(c);
    c << num;
    c >> num;
    return c;
}

[[nodiscard]] constexpr uint32_t utf8_start_value(char c) noexcept
{
    return utf8_start_value(static_cast<uint8_t>(c));
}

/** Add a unicode code point to a UTF-16 string.
 *
 * @param r[out] String to append the unicode code point to.
 * @param code The code point to add.
 */
inline void utf8_to_utf32_add(std::u32string &r, uint32_t code) noexcept
{
    tt_axiom(code < 0x110000);
    tt_axiom(code < 0xd800 or code >= 0xe000);

    r += static_cast<char32_t>(code);
}

/** Add a unicode code point to a UTF-16 string.
 *
 * @param r[out] String to append the unicode code point to.
 * @param code The code point to add.
 */
inline void utf8_to_utf16_add(std::u16string &r, uint32_t code) noexcept
{
    tt_axiom(code < 0x110000);
    tt_axiom(code < 0xd800 or code >= 0xe000);

    if (code < 0x10000) {
        r += static_cast<char32_t>(code);
    } else {
        [[unlikely]] code -= 0x10000;
        r += static_cast<char32_t>(0xd800 + (code >> 10));
        r += static_cast<char32_t>(0xdc00 + code & 0x3ff);
    }
}

}

/** Replace code-units with the replacement character.
 *
 * All the code-units in the given range are replaced with a single replacement character.
 * This function may invalidate iterators and references to the given string.
 *
 * @param r [in,out] The UTF-8 string to modify
 * @param first The iterator where to start replacing code-units.
 * @param n The number of code-units to replace.
 * @return Iterator positioned after the replacement character.
 */
inline std::string::iterator utf8_replacement_character(std::string &r, std::string::iterator first, size_t n) noexcept
{
    if (n < 3) {
        first = r.insert(first, '\0', 3 - n);
    }

    *(first++) = 0xef;
    *(first++) = 0xbf;
    *(first++) = 0xbd;

    if (n > 3) {
        first = r.erase(first, std::advance(first, n - 3));
    }
    return first
}

/** Replace code-units with the replacement character.
 *
 * All the code-units in the given range are replaced with a single replacement character.
 * This function may invalidate iterators and references to the given string.
 *
 * @param r [in,out] The UTF-8 string to modify
 * @param first The iterator where to start replacing code-units.
 * @param last One beyond the last code-unit to replace.
 * @return Iterator positioned after the replacement character.
 */
constexpr std::string::iterator utf8_replacement_character(std::string &r, std::string::iterator first, std::string::iterator last) noexcept
{
    return utf8_replacement_character(r, first, std::distance(last));
}

/** Santize a UTF-8 string.
 *
 * Replaces all encoding errors with the unicode replacement character.
 * The following error codintions are detected:
 *  - Continuation codes without start.
 *  - Incorrect number of continuation codes after start.
 *  - Invalid code units.
 *  - Overlong encoding.
 *  - Surrogate codes points.
 *  - Code points beyond U+10ffff.
 *
 * @param r [in,out] The UTF-8 string to sanitize.
 */
constexpr void utf8_sanatise(std::string &r) noexcept
{
    auto it = std::begin(r);
    auto start = it;

    uint8_t todo = 0;
    uint32_t value = 0;
    while (it != std::end(r)) {
        auto c = static_cast<uint8_t>(*it);

        if (not todo) {
            start = it++;
            if (c < 0x80) {
                // ASCII
                todo = 0;
                value = 0;
            } else if (c < 0xc0) {
                // Continuation
                todo = 0;
                value = 0;
                [[unlikely]] it = utf8_replacement_character(r, start, it);
            } else if (c < 0xe0) {
                // Start + 1
                todo = 1;
                value = c & 0x1f;
            } else if (c < 0xf0) {
                // Start + 2
                todo = 2;
                value = c & 0x0f;
            } else {
                // Start + 3
                todo = 3;
                value = c & 0x07;
            }

        } else if (c >= 0x80 and c < 0xc0) {
            // Continuation
            ++it;
            value <<= 6;
            value |= c & 0x3f;

            // Check if this is the last continuation.
            if (not --todo) {
                ttlet num = std::distance(start, it);
                // clang-format off
                if (
                    (num == 2 and value < 0x80) or
                    (num == 3 and value < 0x800) or
                    (num == 4 and value < 0x10000) or
                    (value >= 0xd800 and value <= 0xdfff) or
                    (value >= 0x10'ffff)) {
                    // overlong encoding, surragetes or overflow..
                    [[unlikely]] it = utf8_replacement_character(r, start, it);
                }
                // clang-format on
            }

        } else {
            // Don't include the invalid continuation, process it again as a start character.
            todo = 0;
            value = 0;
            [[unlikely]] it = utf8_replacement_character(r, start, it);
        }
    }

    if (todo) {
        [[unlikely]] utf8_replacement_character(r, start, r.end());
    }

    return r;
}

/** Santize a UTF-8 string.
 *
 * Replaces all encoding errors with the unicode replacement character.
 * The following error codintions are detected:
 *  - Continuation codes without start.
 *  - Incorrect number of continuation codes after start.
 *  - Invalid code units.
 *  - Overlong encoding.
 *  - Surrogate codes points.
 *  - Code points beyond U+10ffff.
 *
 * @param rhs The UTF-8 string to sanitize.
 * @return The sanitized UTF-8 string.
 */
[[nodiscard]] constexpr std::string utf8_sanitize(std::string &&rhs) noexcept
{
    auto r = std::move(rhs);
    utf8_sanitize(r);
    return r;
}

/** Convert a valid UTF-8 string to UTF-32.
 *
 * @pre The given UTF-8 string must be valid.
 * @param rhs The UTF-8 string to decode
 * @return The resulting UTF-32 string.
 */
[[nodiscard]] inline std::u32string utf8_to_utf32(std::string_view rhs) noexcept
{
    auto r = std::u32string{};
    r.reserve(detail::num_code_points(rhs));

    auto it = std::begin(rhs);
    ttlet last = std::end(rhs);

    if (it != last) {
        auto value = detail::utf8_start_value(*it);
        while (it != last) {
            ttlet c = static_cast<uint8_t>(*(it++));
            if ((c & 0b1100'000) == 0b1000'0000) {
                // Continuation
                value <<= 6;
                value |= c & 0x3f;

            } else {
                detail::utf8_to_utf32_add(value);
                value = detail::utf8_start_value(c);
            }
        }

        detail::utf8_to_utf32_add(value);
    }

    return r;
}

/** Convert a valid UTF-8 string to UTF-16.
 *
 * @pre The given UTF-8 string must be valid.
 * @param rhs The UTF-8 string to decode
 * @return The resulting UTF-16 string.
 */
[[nodiscard]] inline std::u16string utf8_to_utf16(std::string_view rhs) noexcept
{
    auto r = std::u16string{};
    r.reserve(detail::num_code_points(rhs));

    auto it = std::begin(rhs);
    ttlet last = std::end(rhs);

    if (it != last) {
        auto value = detail::utf8_start_value(*it);
        while (it != last) {
            ttlet c = static_cast<uint8_t>(*(it++));
            if ((c & 0b1100'000) == 0b1000'0000) {
                // Continuation
                value <<= 6;
                value |= c & 0x3f;

            } else {
                detail::utf8_to_utf16_add(value);
                value = detail::utf8_start_value(c);
            }
        }

        detail::utf8_to_utf16_add(value);
    }

    return r;
}

[[nodiscard]] inline std::sting utf32_to_utf8(std::u32string_view rhs) noexcept
{
    auto r = std::string{};
    r.reserve(num_utf8_code_units(rhs));

    for (ttlet c: rhs) {
        tt_axiom(c < 0x11'0000);
        tt_axiom(c < 0xd800 or c >= 0x1'0000);

        if (c < 0x80) {
            r += static_cast<char>(c);
        } else if (c < 0x800) {
            r += static_cast<char>((c >> 6) | 0xc0);
            r += static_cast<char>((c & 0x3f) | 0x80);
        } else if (c < 0x1'0000) {
            r += static_cast<char>((c >> 12) | 0xe0);
            r += static_cast<char>(((c >> 6) & 0x3f) | 0x80);
            r += static_cast<char>((c & 0x3f) | 0x80);
        } else {
            r += static_cast<char>((c >> 18) | 0xe0);
            r += static_cast<char>(((c >> 12) & 0x3f) | 0x80);
            r += static_cast<char>(((c >> 6) & 0x3f) | 0x80);
            r += static_cast<char>((c & 0x3f) | 0x80);
        }
    }

    return r;
}

[[nodiscard]] inline std::sting utf16_to_utf8(std::u16string_view rhs) noexcept
{
    auto r = std::string{};
    r.reserve(num_utf8_code_units(rhs));

    uint32_t value;
    for (ttlet c: rhs) {
        tt_axiom(c < 0x11'0000);
        tt_axiom(c < 0xd800 or c >= 0x1'0000);

        if (c < 0x80) {
            r += static_cast<char>(c);
        } else if (c < 0x800) {
            r += static_cast<char>((c >> 6) | 0xc0);
            r += static_cast<char>((c & 0x3f) | 0x80);
        } else if (c < 0xd800 or c >= 0xe000) {
            r += static_cast<char>((c >> 12) | 0xe0);
            r += static_cast<char>(((c >> 6) & 0x3f) | 0x80);
            r += static_cast<char>((c & 0x3f) | 0x80);
        } else if (c < 0xdc00) {
            value = (c - 0xd800) << 10;
        } else {
            value |= c - 0xdc00;
            value += 0x1'0000;

            r += static_cast<char>((value >> 18) | 0xe0);
            r += static_cast<char>(((value >> 12) & 0x3f) | 0x80);
            r += static_cast<char>(((value >> 6) & 0x3f) | 0x80);
            r += static_cast<char>((value & 0x3f) | 0x80);
        }
    }

    return r;
}

}

