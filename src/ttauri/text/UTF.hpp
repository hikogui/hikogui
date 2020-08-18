// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include "../endian.hpp"
#include "../CP1252.hpp"
#include <type_traits>
#include <iterator>
#include <bit>

namespace tt {

/** Convert a UTF-16 encoded code-point to a UTF-32 encoded code-point
 *
 * It is undefined behavior when the iterator does not point to a valid
 * and complete UTF-16 encoded code-point.
 */
template<typename Iterator>
[[nodiscard]] constexpr char32_t utf16_to_utf32(Iterator it) noexcept
{
    using value_type = std::remove_cv_t<typename std::iterator_traits<Iterator>::value_type>;
    static_assert(std::is_same_v<value_type, char16_t>, "Iterator must point to a char16_t");

    ttlet first = *it;
    if (first <= 0xd7ff || first >= 0xe000) {
        return first;

    } else {
        tt_assume2(first <= 0xdbff, "Expecting the high surrogate");
        ttlet second = *(++it);
        tt_assume2(second >= 0xdc00 && second <= 0xdfff, "Expecting the low surrogate");

        return ((static_cast<char32_t>(first - 0xd800) << 10) | (static_cast<char32_t>(second - 0xdc00))) + 0x01'0000;
    }
}

/** Convert a UTF-8 encoded code-point to a UTF-32 encoded code-point
 *
 * It is undefined behavior when the iterator does not point to a valid
 * and complete UTF-8 encoded code-point.
 */
template<typename Iterator>
[[nodiscard]] constexpr char32_t utf8_to_utf32(Iterator it) noexcept
{
    using value_type = std::remove_cv_t<typename std::iterator_traits<Iterator>::value_type>;
    static_assert(std::is_same_v<value_type, char8_t>, "Iterator must point to a char8_t");

    auto cp = char32_t{0};

    ttlet cu = *it;
    if (cu <= 0x7f) {
        return cu;

    } else if (cu <= 0xdf) {
        tt_assume2(cu >= 0xc0, "UTF-8 encoded code-point can not start with continuation code-units");
        cp = static_cast<char32_t>(cu & 0x1f) << 6;
        cp |= *(++it) & 0x3f;
        tt_assume2(cp >= 0x0080 && cp < 0x07ff, "UTF-8 Overlong encoding");
        return cp;

    } else if (cu <= 0xef) {
        cp = static_cast<char32_t>(cu & 0x0f) << 6;
        cp |= static_cast<char32_t>(*(++it) & 0x3f) << 6;
        cp |= *(++it) & 0x3f;
        tt_assume2(cp >= 0x0800 && cp < 0xffff, "UTF-8 Overlong encoding");
        tt_assume2(!(cp >= 0xd800 && cp <= 0xdfff), "UTF-8 Must not encode surrogates");
        return cp;

    } else {
        tt_assume2(cu <= 0xf7, "UTF8 encoded code-point must have a valid start code-unit");
        cp = static_cast<char32_t>(cu & 0x07) << 6;
        cp |= static_cast<char32_t>(*(++it) & 0x3f) << 6;
        cp |= static_cast<char32_t>(*(++it) & 0x3f) << 6;
        cp |= *(++it) & 0x3f;
        tt_assume2(cp >= 0x100000 && cp < 0x10ffff, "UTF-8 Overlong encoding");
        return cp;
    }
}

/** Convert a single UTF-8 encoded code-point to a UTF-32 encoded code-point
 *
 * This function will try to create a code-point from the given UTF-8 substring.
 * If an invalid code-unit is found in the data an attempt is made to use the CP-1252 character encoding.
 *
 * @param [in,out] it Location of first UTF-8 code-unit of a UTF-8 encoded code-point.
 *           After calling this function `it` will always advance beyond the current code-point.
 * @param last The location one beyond the UTF-8 string. This allows detection of incomplete encoded
 *             code point at the end of the string
 * @param [out] code_point The decoded code-point: decoded from UTF-8 or the CP-1252 fallback.
 * @return True if a valid UTF-8 encoded code-unit was decoded.
 */
template<typename Iterator>
constexpr bool utf8_to_utf32(Iterator &it, Iterator last, char32_t &code_point) noexcept
{
    using value_type = std::remove_cv_t<typename std::iterator_traits<Iterator>::value_type>;
    static_assert(std::is_same_v<value_type, char8_t>, "Iterator must point to a char8_t");

    auto continuation_count = 0;
    auto first_cu = *(it++);
    if (first_cu <= 0x7f) {
        code_point = first_cu;
        return true;

    } else if (first_cu <= 0xbf) {
        // Invalid continuation character.
        code_point = CP1252_to_UTF32(static_cast<char>(first_cu));
        return false;

    } else if (first_cu <= 0xdf) {
        code_point = static_cast<char32_t>(first_cu & 0x1f);
        continuation_count = 1;

    } else if (first_cu <= 0xef) {
        code_point = static_cast<char32_t>(first_cu & 0x0f);
        continuation_count = 2;

    } else if (first_cu <= 0xf7) {
        code_point = static_cast<char32_t>(first_cu & 0x07);
        continuation_count = 3;

    } else {
        // Invalid code-unit
        code_point = CP1252_to_UTF32(static_cast<char>(first_cu));
        return false;
    }

    auto old_it = it;
    for (int i = 0; i != continuation_count; ++i) {
        if (tt_unlikely(it == last || (*it & 0xc0) != 0x80)) {
            // No continuation character, or at end of string.
            code_point = CP1252_to_UTF32(static_cast<char>(first_cu));
            it = old_it;
            return false;
        }

        code_point <<= 6;
        code_point = *(it++) & 0x3f;
    }

    if ((code_point >= 0xd800 && code_point <= 0xdfff) || // Surrogate pair
        (continuation_count == 1 && code_point < 0x0080) || // Overlong
        (continuation_count == 2 && code_point < 0x0800) || // Overlong
        (continuation_count == 3 && code_point < 0x10000) // Overlong
    ) {
        // Surrogate pair
        code_point = CP1252_to_UTF32(static_cast<char>(first_cu));
        it = old_it;
        return false;
    }

    return true;
}

/** Convert a UTF-32 encoded code point to a UTF-16 encoded code point.
 * It is undefined behavior when the code-point is outside the Unicode range or if it is a surrogate-code.
 */
template<typename Iterator>
constexpr void utf32_to_utf16(char32_t code_point, Iterator &it) noexcept
{
    using value_type = std::remove_cv_t<typename std::iterator_traits<Iterator>::value_type>;
    static_assert(std::is_same_v<value_type, char16_t>, "Iterator must point to a char16_t");

    if (code_point <= 0xffff) {
        tt_assume2(!(code_point >= 0xd800 && code_point <= 0xdfff), "Code Point must not be a surrogate-code");
        *(it++) = static_cast<char16_t>(code_point);

    } else {
        tt_assume2(code_point <= 0x10ffff, "Code Point must be in range of the 17 planes");

        code_point -= 0x10000;
        *(it++) = static_cast<char16_t>(code_point >> 10) | 0xd800;
        *(it++) = static_cast<char16_t>(code_point) & 0x03ff | 0xdc00;
    }
}

/** Convert a UTF-32 encoded code point to a UTF-8 encoded code point.
 * It is undefined behavior when the code-point is outside the Unicode range or if it is a surrogate-code.
 */
template<typename Iterator>
constexpr void utf32_to_utf8(char32_t code_point, Iterator &it) noexcept
{
    if (code_point <= 0x7f) {
        *(it++) = static_cast<char8_t>(code_point);

    } else if (code_point <= 0x07ff) {
        *(it++) = static_cast<char8_t>(code_point >> 6) | 0xc0;
        *(it++) = static_cast<char8_t>(code_point) & 0x3f | 0x80;

    } else if (code_point <= 0xffff) {
        tt_assume2(!(code_point >= 0xd800 && code_point <= 0xdfff), "Code Point must not be a surrogate");
        *(it++) = static_cast<char8_t>(code_point >> 12) | 0xe0;
        *(it++) = static_cast<char8_t>(code_point >> 6) & 0x3f | 0x80;
        *(it++) = static_cast<char8_t>(code_point) & 0x3f | 0x80;

    } else {
        tt_assume2(code_point <= 0x10ffff, "Code Point must be in range of the 17 planes");
        *(it++) = static_cast<char8_t>(code_point >> 18) | 0xf0;
        *(it++) = static_cast<char8_t>(code_point >> 12) & 0x3f | 0x80;
        *(it++) = static_cast<char8_t>(code_point >> 6) & 0x3f | 0x80;
        *(it++) = static_cast<char8_t>(code_point) & 0x3f | 0x80;
    }
}

/** Make a u16string from a container containing UTF-16 data.
 * The container may be a byte-array,string,span; or a array of words.
 *
 * The buffer is converted as-is, when invalid code-units are contained in the
 * buffer these invalid code-units will be copied. Use `sanitize_utf16()` to
 * get a u16string with valid UTF-16.
 *
 * @tparam Container type of the buffer with UTF-16 data
 * @tparam ContainerEndian Endian of the UTF-16 data.
 * @param rhs The buffer of UTF-16 data
 * @return A u16string containing the UTF-16 data.
 */
template<typename Container, std::endian Endian = std::endian::native>
[[nodiscard]] std::u16string make_u16string(Container const &rhs) noexcept
{
    auto r = std::u16string{};

    if constexpr (sizeof(Container::value_type) == 1) {
        // A byte array of some sorts, copy each pair of bytes.
        r.reserve((size(rhs) + 1) / 2);
        for (ssize_t i = 0; i < ssize(rhs); i += 2) {
            if constexpr (Endian == std::endian::little) {
                r += static_cast<char16_t>(rhs[i]) << 8 | static_cast<char16_t>(rhs[i + 1]);
            } else {
                r += static_cast<char16_t>(rhs[i]) | static_cast<char16_t>(rhs[i + 1]) << 8;
            }
        }
        if (size(rhs) % 2 == 1) {
            // Odd number of bytes.
            r += 0xfffd;
        }

    } else {
        // An array of 16-bits or large.
        r.reserve(size(rhs));
        for (auto &&c : rhs) {
            r += Endian == std::endian::native ? static_cast<char16_t>(c) : static_cast<char16_t>(byte_swap(c));
        }
    }

    return r;
}

/** Sanitize a UTF-16 string so it contains only valid encoded Unicode code points.
 *
 * This function will replace invalid code units with the unicode-replacement-character 0xfffd.
 * This will also byte-swap based on byte-order-marks anywhere in the string.
 *
 * @param rhs A string with possibly invalid UTF-16 code units.
 * @return A valid UTF-16 string.
 */
[[nodiscard]] inline std::u16string sanitize_u16string(std::u16string &&rhs) noexcept
{
    auto r = std::move(rhs);

    auto swap_endian = false;

    ttlet length = size(r);
    auto i = 0;
    while (i != length) {
        auto code_unit = r[i];
        if (swap_endian) {
            code_unit = r[i] = byte_swap(code_unit);
        }

        if (code_unit == 0xfffe) {
            // Byte-swapped BOM.
            swap_endian = !swap_endian;
            // Don't increment iterator

        } else if (code_unit >= 0xd800 && code_unit <= 0xdbff) {
            // Found high surrogate.
            auto old_i = i++;
            if (i != length) {
                auto next_code_unit = r[i];
                if (next_code_unit >= 0xdc00 && next_code_unit <= 0xdfff) {
                    // valid surrogate pair
                    ++i;

                } else {
                    // Invalid surrogate pair.
                    // Replace the high-surrogate, then sync back to the current code-unit.
                    r[old_i] = char16_t{0xfffd};
                }

            } else {
                // High surrogate at end of string.
                r[old_i] = char16_t{0xfffd};
            }

        } else if (code_unit >= 0xdc00 && code_unit <= 0xdfff) {
            // Found invalid low surrogate.
            r[i++] = char16_t{0xfffd};

        } else {
            ++i;
        }
    }

    return r;
}

template<typename Container>
[[nodiscard]] std::u8string make_u8string(Container const &rhs) noexcept
{
    auto r = std::u8string{};

    // An array of 16-bits or large.
    r.reserve(size(rhs));
    for (auto &&c : rhs) {
        r += static_cast<char8_t>(c);
    }

    return r;
}

/** Sanitize a UTF-8 string so it contains only valid encoded Unicode code points.
 *
 * This function will replace invalid code units by treating these as CP-1252 encoded
 * characters and re-encoding them as valid UTF-8.
 *
 * @param rhs A string with possibly invalid UTF-8 code units.
 * @return A valid UTF-8 string.
 */
[[nodiscard]] inline std::u8string sanitize_u8string(std::u8string &&rhs) noexcept
{
    auto r = std::move(rhs);

    ttlet first = begin(rhs);
    ttlet last = end(rhs);

    auto code_point = char32_t{};
    auto valid = true;
    auto old_it = first;
    for (auto it = first; valid && it != last;) {
        old_it = it;
        valid &= utf8_to_utf32(it, last, code_point);
    }

    if (valid) {
        return r;
    }

    // Copy the valid UTF-8 code units and prepare for
    // re-encoding the rest of the string.
    auto tmp = std::u8string{begin(r), old_it};
    tmp.reserve(size(r));

    // Add the last decoded code point.
    auto tmp_i = std::back_inserter(tmp);
    utf32_to_utf8(code_point, tmp_i);

    // Re-encode the rest of the string.
    for (auto it = old_it + 1; it != last;) {
        utf8_to_utf32(it, last, code_point);
        utf32_to_utf8(code_point, tmp_i);
    }

    std::swap(r, tmp);
    return r;
}

} // namespace tt