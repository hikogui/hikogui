// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

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
 * Invalid code units, such as unpaired surrogates are converted to U+fffd.
 *
 * @tparam Iterator A LegacyInputIterator
 * @param [in,out] it An iterator pointing to the first UTF-16 code unit of a code point.
 *                    After the call the iterator points beyond the code point.
 * @param last An iterator pointing one beyond the last UTF-16 code unit.
 * @return The encoded code point
 */
template<typename Iterator, typename Sentinel>
[[nodiscard]] constexpr char32_t utf16_to_utf32(Iterator &it, Sentinel last) noexcept
{
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    static_assert(
        std::is_same_v<value_type, char16_t> or (std::is_same_v<value_type, wchar_t> and sizeof(wchar_t) == 2),
        "Iterator must point to a char16_t or 16 bit wchar_t");

    ttlet first = static_cast<char16_t>(*(it++));
    if (first <= 0xd7ff or first >= 0xe000) {
        return first;

    } else if (first <= 0xdbff) {
        if (it == last or static_cast<char16_t>(*it) < 0xdc00 or static_cast<char16_t>(*it) > 0xdfff) {
            // Unpaired high surrogate. Don't increment `it` yet.
            [[unlikely]] return 0xfffd;

        } else {
            // High surrogate.
            ttlet second = static_cast<char16_t>(*(it++));
            return ((static_cast<char32_t>(first - 0xd800) << 10) | (static_cast<char32_t>(second - 0xdc00))) + 0x01'0000;
        }

    } else if (first <= 0xdfff) {
        // Unpaired low surrogate.
        return 0xfffd;

    } else {
        return first;
    }
}

/** Convert a UTF-8 encoded code-point to a UTF-32 encoded code-point
 *
 * It is undefined behavior when the iterator does not point to a valid
 * and complete UTF-8 encoded code-point.
 *
 * @tparam Iterator A LegacyInputIterator
 * @param [in,out] it An iterator pointing to the first UTF-8 code unit of a code point.
 *                    After the call the iterator points beyond the code point.
 * @return The encoded code point
 */
template<typename Iterator>
[[nodiscard]] constexpr char32_t utf8_to_utf32(Iterator &it) noexcept
{
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    static_assert(std::is_same_v<value_type, char8_t>, "Iterator must point to a char8_t");

    auto cp = char32_t{0};

    ttlet cu = *(it++);
    if (cu <= 0x7f) {
        return static_cast<char32_t>(cu);

    } else if (cu <= 0xdf) {
        tt_axiom(cu >= 0xc0, "UTF-8 encoded code-point can not start with continuation code-units");
        cp = static_cast<char32_t>(cu & 0x1f);
        cp <<= 6;
        cp |= static_cast<char32_t>(*(it++) & 0x3f);
        tt_axiom(cp >= 0x0080 && cp <= 0x07ff, "UTF-8 Overlong encoding");
        return cp;

    } else if (cu <= 0xef) {
        cp = static_cast<char32_t>(cu & 0x0f);
        cp <<= 6;
        cp |= static_cast<char32_t>(*(it++) & 0x3f);
        cp <<= 6;
        cp |= static_cast<char32_t>(*(it++) & 0x3f);
        tt_axiom(cp >= 0x0800 && cp <= 0xffff, "UTF-8 Overlong encoding");
        tt_axiom(!(cp >= 0xd800 && cp <= 0xdfff), "UTF-8 Must not encode surrogates");
        return cp;

    } else {
        tt_axiom(cu <= 0xf7, "UTF8 encoded code-point must have a valid start code-unit");
        cp = static_cast<char32_t>(cu & 0x07);
        cp <<= 6;
        cp |= static_cast<char32_t>(*(it++) & 0x3f);
        cp <<= 6;
        cp |= static_cast<char32_t>(*(it++) & 0x3f);
        cp <<= 6;
        cp |= static_cast<char32_t>(*(it++) & 0x3f);
        tt_axiom(cp >= 0x100000 && cp <= 0x10ffff, "UTF-8 Overlong encoding");
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
    using value_type = typename std::iterator_traits<Iterator>::value_type;
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
        if (it == last || (*it & 0xc0) != 0x80) {
            // No continuation character, or at end of string.
            [[unlikely]] code_point = CP1252_to_UTF32(static_cast<char>(first_cu));
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
 *
 * @tparam Iterator A LegacyOutputIterator
 * @param code_point The code point to encode.
 * @param [in,out] it An iterator pointing to where the UTF-16 code units should be inserted.
 *                    After the call the iterator points beyond the code point.
 */
template<typename BackInsertIterator>
constexpr void utf32_to_utf16(char32_t code_point, BackInsertIterator &it) noexcept
{
    using value_type = typename BackInsertIterator::container_type::value_type;
    static_assert(sizeof(value_type) == 2, "Iterator must point to a two byte character type");

    if (code_point <= 0xffff) {
        tt_axiom(!(code_point >= 0xd800 && code_point <= 0xdfff), "Code Point must not be a surrogate-code");
        *(it++) = static_cast<value_type>(code_point);

    } else {
        tt_axiom(code_point <= 0x10ffff, "Code Point must be in range of the 17 planes");

        code_point -= 0x10000;
        *(it++) = static_cast<value_type>(code_point >> 10) | 0xd800;
        *(it++) = static_cast<value_type>(code_point) & 0x03ff | 0xdc00;
    }
}

/** Convert a UTF-32 encoded code point to a UTF-8 encoded code point.
 *
 * Invalid code-points are converted to U+fffd.
 *
 * Either the full code-point is added to the @a it, or when it doesn't fit
 * before @a nul characters are added. When @a it == @a last nothing will be inserted.
 *
 * @tparam Iterator A LegacyOutputIterator
 * @param code_point The code point to encode.
 * @param [in,out] it An iterator pointing to where the UTF-8 code units should be inserted.
 *                    After the call the iterator points beyond the code point.
 */
template<typename Iterator>
constexpr void utf32_to_utf8(char32_t code_point, Iterator &it) noexcept
{
    if ((code_point >= 0xd800 and code_point <= 0xdfff) or code_point > 0x10ffff) {
        // Code point is invalid.
        code_point = 0xfffd;
    }

    if (code_point <= 0x7f) {
        *(it++) = static_cast<uint8_t>(code_point);

    } else if (code_point <= 0x07ff) {
        *(it++) = static_cast<uint8_t>(code_point >> 6) | 0xc0;
        *(it++) = static_cast<uint8_t>(code_point) & 0x3f | 0x80;

    } else if (code_point <= 0xffff) {
        *(it++) = static_cast<uint8_t>(code_point >> 12) | 0xe0;
        *(it++) = static_cast<uint8_t>(code_point >> 6) & 0x3f | 0x80;
        *(it++) = static_cast<uint8_t>(code_point) & 0x3f | 0x80;

    } else {
        *(it++) = static_cast<uint8_t>(code_point >> 18) | 0xf0;
        *(it++) = static_cast<uint8_t>(code_point >> 12) & 0x3f | 0x80;
        *(it++) = static_cast<uint8_t>(code_point >> 6) & 0x3f | 0x80;
        *(it++) = static_cast<uint8_t>(code_point) & 0x3f | 0x80;
    }
}

/** Convert a UTF-32 encoded code point to a UTF-8 encoded code point.
 * 
 * Invalid code-points are converted to U+fffd.
 * 
 * Either the full code-point is added to the @a it, or when it doesn't fit
 * before @a nul characters are added. When @a it == @a last nothing will be inserted.
 *
 * @tparam Iterator A LegacyOutputIterator
 * @param code_point The code point to encode.
 * @param [in,out] it An iterator pointing to where the UTF-8 code units should be inserted.
 *                    After the call the iterator points beyond the code point.
 * @param last One beyond the last position to insert.
 */
template<typename Iterator, typename Sentinel>
constexpr void utf32_to_utf8(char32_t code_point, Iterator &it, Sentinel last) noexcept
{
    if ((code_point >= 0xd800 and code_point <= 0xdfff) or code_point > 0x10ffff) {
        // Code point is invalid.
        code_point = 0xfffd;
    }

    // clang-format off
    if (
        (it == last) or
        (code_point >= 0x80 and it + 1 == last) or
        (code_point >= 0x800 and it + 2 == last) or
        (code_point >= 0x10000 and it + 3 == last) 
    ) {
        // No space for writing. Fill rest with nul chars.
        while (it != last) {
            *(it++) = uint8_t{0};
        }
    } else {
        utf32_to_utf8(code_point, it);
    }
    // clang-format on
}

/** Sanitize a UTF-32 string so it contains only valid encoded Unicode code points.
 *
 * This function will replace invalid code units with the unicode-replacement-character 0xfffd.
 *
 * @param rhs A string with possibly invalid UTF-32 code units.
 * @return A valid UTF-32 string.
 */
[[nodiscard]] inline std::u32string sanitize_u32string(std::u32string &&rhs) noexcept
{
    auto r = std::move(rhs);

    for (char32_t &c : r) {
        if (c > 0x10'ffff || (c >= 0xd800 && c <= 0xdfff)) {
            c = U'\ufffd';
        }
    }

    return r;
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
        // An array of 16-bits or larger.
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

    // An array of 8-bits.
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

namespace detail {

template<typename StringT>
[[nodiscard]] inline StringT to_u8string(std::u16string_view const &rhs) noexcept
{
    auto r = StringT{};
    r.reserve(rhs.size());
    auto r_it = std::back_inserter(r);

    for (auto it = std::begin(rhs); it != std::end(rhs);) {
        ttlet c32 = utf16_to_utf32(it, std::end(rhs));
        utf32_to_utf8(c32, r_it);
    }
    return r;
}

template<typename StringT>
[[nodiscard]] inline StringT to_u8string(std::u32string_view const &rhs) noexcept
{
    auto r = StringT{};
    r.reserve(rhs.size());
    auto r_it = std::back_inserter(r);

    for (auto c32 : rhs) {
        utf32_to_utf8(c32, r_it);
    }
    return r;
}

template<typename StringT>
[[nodiscard]] inline StringT to_u8string(std::wstring_view const &rhs) noexcept
{
    if constexpr (sizeof(std::wstring::value_type) == 2) {
        auto s16 = sanitize_u16string(std::u16string{reinterpret_cast<char16_t const *>(rhs.data()), rhs.size()});
        return to_u8string<StringT>(std::move(s16));
    } else {
        auto s32 = sanitize_u32string(std::u32string{reinterpret_cast<char32_t const *>(rhs.data()), rhs.size()});
        return to_u8string<StringT>(std::move(s32));
    }
}

} // namespace detail

/** UTF-8 to string conversion.
 * It is undefined behavior if the given string is not a valid UTF-8 string.
 *
 * @param rhs The given valid UTF-8 encoded string.
 * @return A UTF-8 encoded string.
 */
[[nodiscard]] inline std::string to_string(std::u8string_view const &rhs) noexcept
{
    return std::string{reinterpret_cast<char const *>(rhs.data()), rhs.size()};
}

/** UTF-16 string to UTF-8 string conversion.
 * It is undefined behavior if the given string is not a valid UTF-16 string.
 *
 * @param rhs The given valid UTF-16 encoded string.
 * @return A UTF-8 encoded string.
 */
[[nodiscard]] inline std::u8string to_u8string(std::u16string_view const &rhs) noexcept
{
    return detail::to_u8string<std::u8string>(rhs);
}

/** UTF-16 string to UTF-8 string conversion.
 * It is undefined behavior if the given string is not a valid UTF-16 string.
 *
 * @param rhs The given valid UTF-16 encoded string.
 * @return A UTF-8 encoded string.
 */
[[nodiscard]] inline std::string to_string(std::u16string_view const &rhs) noexcept
{
    return detail::to_u8string<std::string>(rhs);
}

/** UTF-32 string to UTF-8 string conversion.
 * It is undefined behavior if the given string is not a valid UTF-32 string.
 *
 * @param rhs The given valid UTF-32 encoded string.
 * @return A UTF-8 encoded string.
 */
[[nodiscard]] inline std::u8string to_u8string(std::u32string_view const &rhs) noexcept
{
    return detail::to_u8string<std::u8string>(rhs);
}

/** UTF-32 string to UTF-8 string conversion.
 * It is undefined behavior if the given string is not a valid UTF-32 string.
 *
 * @param rhs The given valid UTF-32 encoded string.
 * @return A UTF-8 encoded string.
 */
[[nodiscard]] inline std::string to_string(std::u32string_view const &rhs) noexcept
{
    return detail::to_u8string<std::string>(rhs);
}

/** UTF-8 string to UTF-16 string conversion.
 * It is undefined behavior if the given string is not a valid UTF-8 string.
 *
 * @param rhs The given valid UTF-8 encoded string.
 * @return A UTF-16 encoded string.
 */
[[nodiscard]] inline std::u16string to_u16string(std::u8string_view const &rhs) noexcept
{
    auto r = std::u16string{};
    r.reserve(rhs.size());
    auto r_it = std::back_inserter(r);

    for (auto it = std::begin(rhs); it != std::end(rhs);) {
        ttlet c32 = utf8_to_utf32(it);
        utf32_to_utf16(c32, r_it);
    }
    return r;
}

/** UTF-32 string to UTF-16 string conversion.
 * It is undefined behavior if the given string is not a valid UTF-32 string.
 *
 * @param rhs The given valid UTF-32 encoded string.
 * @return A UTF-16 encoded string.
 */
[[nodiscard]] inline std::u16string to_u16string(std::u32string_view const &rhs) noexcept
{
    auto r = std::u16string{};
    r.reserve(rhs.size());
    auto r_it = std::back_inserter(r);

    for (auto c32 : rhs) {
        utf32_to_utf16(c32, r_it);
    }
    return r;
}

/** UTF-8 string to UTF-32 string conversion.
 * It is undefined behavior if the given string is not a valid UTF-8 string.
 *
 * @param rhs The given valid UTF-8 encoded string.
 * @return A UTF-32 encoded string.
 */
[[nodiscard]] inline std::u32string to_u32string(std::u8string_view const &rhs) noexcept
{
    auto r = std::u32string{};
    r.reserve(rhs.size());

    for (auto it = std::begin(rhs); it != std::end(rhs);) {
        r += utf8_to_utf32(it);
    }
    return r;
}

/** UTF-16 string to UTF-32 string conversion.
 * It is undefined behavior if the given string is not a valid UTF-16 string.
 *
 * @param rhs The given valid UTF-16 encoded string.
 * @return A UTF-32 encoded string.
 */
[[nodiscard]] inline std::u32string to_u32string(std::u16string_view const &rhs) noexcept
{
    auto r = std::u32string{};
    r.reserve(rhs.size());

    for (auto it = std::begin(rhs); it != std::end(rhs);) {
        r += utf16_to_utf32(it, std::end(rhs));
    }
    return r;
}

/** Convert a string to a UTF-8 encoded string.
 * The given string should be UTF-8 encoded, but may contain invalid code-units.
 *
 * @param rhs A UTF-8 encoded string, which may be invalid.
 * @return A valid UTF-8 encoded string.
 */
[[nodiscard]] inline std::u8string to_u8string(std::string_view const &rhs) noexcept
{
    return sanitize_u8string(std::u8string{reinterpret_cast<char8_t const *>(rhs.data()), rhs.size()});
}

/** Convert a string to a UTF-16 encoded string.
 * The given string should be UTF-8 encoded, but may contain invalid code-units.
 *
 * @param rhs A UTF-8 encoded string, which may be invalid.
 * @return A valid UTF-16 encoded string.
 */
[[nodiscard]] inline std::u16string to_u16string(std::string_view const &rhs) noexcept
{
    return to_u16string(sanitize_u8string(std::u8string{reinterpret_cast<char8_t const *>(rhs.data()), rhs.size()}));
}

/** Convert a string to a UTF-32 encoded string.
 * The given string should be UTF-8 encoded, but may contain invalid code-units.
 *
 * @param rhs A UTF-8 encoded string, which may be invalid.
 * @return A valid UTF-32 encoded string.
 */
[[nodiscard]] inline std::u32string to_u32string(std::string_view const &rhs) noexcept
{
    return to_u32string(sanitize_u8string(std::u8string{reinterpret_cast<char8_t const *>(rhs.data()), rhs.size()}));
}

/** Convert a wide-string to a UTF-8 encoded string.
 * The given string should be UTF-16 or UTF-32 encoded, but may contain invalid code-units.
 *
 * @param rhs A UTF-16 or UTF-32 encoded string, which may be invalid.
 * @return A valid UTF-8 encoded string.
 */
[[nodiscard]] inline std::u8string to_u8string(std::wstring_view const &rhs) noexcept
{
    return detail::to_u8string<std::u8string>(rhs);
}

/** Convert a wide-string to a UTF-8 encoded string.
 * The given string should be UTF-16 or UTF-32 encoded, but may contain invalid code-units.
 *
 * @param rhs A UTF-16 or UTF-32 encoded string, which may be invalid.
 * @return A valid UTF-8 encoded string.
 */
[[nodiscard]] inline std::string to_string(std::wstring_view const &rhs) noexcept
{
    return detail::to_u8string<std::string>(rhs);
}

/** Convert a wide-string to a UTF-16 encoded string.
 * The given string should be UTF-16 or UTF-32 encoded, but may contain invalid code-units.
 *
 * @param rhs A UTF-16 or UTF-32 encoded string, which may be invalid.
 * @return A valid UTF-16 encoded string.
 */
[[nodiscard]] inline std::u16string to_u16string(std::wstring_view const &rhs) noexcept
{
    if constexpr (sizeof(std::wstring::value_type) == 2) {
        return sanitize_u16string(std::u16string{reinterpret_cast<char16_t const *>(rhs.data()), rhs.size()});
    } else {
        auto s32 = sanitize_u32string(std::u32string{reinterpret_cast<char32_t const *>(rhs.data()), rhs.size()});
        return to_u16string(std::move(s32));
    }
}

/** Convert a wide-string to a UTF-16 encoded string.
 * The given string should be UTF-16 or UTF-32 encoded, but may contain invalid code-units.
 *
 * @param rhs A UTF-16 or UTF-32 encoded string, which may be invalid.
 * @return A valid UTF-32 encoded string.
 */
[[nodiscard]] inline std::u32string to_u32string(std::wstring_view const &rhs) noexcept
{
    if constexpr (sizeof(std::wstring::value_type) == 2) {
        auto s16 = sanitize_u16string(std::u16string{reinterpret_cast<char16_t const *>(rhs.data()), rhs.size()});
        return to_u32string(std::move(s16));
    } else {
        return sanitize_u32string(std::u32string{reinterpret_cast<char32_t const *>(rhs.data()), rhs.size()});
    }
}

/** Convert a UTF-8 encoded string to a wide-string.
 * It is undefined behavior if the given string contains invalid UTF-8 code points.
 *
 * @param rhs A UTF-8 encoded string, which may be invalid.
 * @return A valid wide string.
 */
[[nodiscard]] inline std::wstring to_wstring(std::u8string_view const &rhs) noexcept
{
    if constexpr (sizeof(std::wstring::value_type) == 2) {
        auto s16 = to_u16string(rhs);
        return std::wstring{reinterpret_cast<wchar_t const *>(s16.data()), s16.size()};
    } else {
        auto s32 = to_u32string(rhs);
        return std::wstring{reinterpret_cast<wchar_t const *>(s32.data()), s32.size()};
    }
}

/** Convert a UTF-8 encoded string to a wide-string.
 * The given string should UTF-8 encoded, but may contain invalid code-units.
 *
 * @param rhs A UTF-8 encoded string, which may be invalid.
 * @return A valid wide string.
 */
[[nodiscard]] inline std::wstring to_wstring(std::string_view const &rhs) noexcept
{
    auto s8 = sanitize_u8string(std::u8string{reinterpret_cast<char8_t const *>(rhs.data()), rhs.size()});
    return to_wstring(s8);
}

/** Convert a UTF-16 encoded string to a wide-string.
 * It is undefined behavior if the given string contains invalid UTF-16 code points.
 *
 * @param rhs A UTF-16 encoded string, which may be invalid.
 * @return A valid wide string.
 */
[[nodiscard]] inline std::wstring to_wstring(std::u16string_view const &rhs) noexcept
{
    if constexpr (sizeof(std::wstring::value_type) == 2) {
        return std::wstring{reinterpret_cast<wchar_t const *>(rhs.data()), rhs.size()};
    } else {
        auto s32 = to_u32string(rhs);
        return std::wstring{reinterpret_cast<wchar_t const *>(s32.data()), s32.size()};
    }
}

/** Convert a UTF-32 encoded string to a wide-string.
 * It is undefined behavior if the given string contains invalid UTF-32 code points.
 *
 * @param rhs A UTF-32 encoded string, which may be invalid.
 * @return A valid wide string.
 */
[[nodiscard]] inline std::wstring to_wstring(std::u32string_view const &rhs) noexcept
{
    if constexpr (sizeof(std::wstring::value_type) == 2) {
        auto s16 = to_u16string(rhs);
        return std::wstring{reinterpret_cast<wchar_t const *>(s16.data()), s16.size()};
    } else {
        return std::wstring{reinterpret_cast<wchar_t const *>(rhs.data()), rhs.size()};
    }
}

} // namespace tt
