// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../cast.hpp"
#include <string>
#include <string_view>
#include <bit>
#include <type_traits>
#include <concepts>

namespace tt::inline v1 {
namespace detail {

template<typename CharT>
constexpr void append_code_point(std::basic_string<CharT> &r, uint32_t code_point) noexcept;

template<typename CharT>
constexpr void append_code_point(std::basic_string<CharT> &r, uint32_t code_point) noexcept requires(sizeof(CharT) == 1)
{
    if (code_point < 0x80) {
        r += static_cast<CharT>(code_point);
    } else if (code_point < 0x800) {
        r += static_cast<CharT>((code_point >> 6) | 0xc0);
        r += static_cast<CharT>((code_point & 0x3f) | 0x80);
    } else if (code_point < 0xd800) {
        r += static_cast<CharT>((code_point >> 12) | 0xe0);
        r += static_cast<CharT>(((code_point >> 6) & 0x3f) | 0x80);
        r += static_cast<CharT>((code_point & 0x3f) | 0x80);
    } else if (code_point < 0xe000) {
        append_code_point(r, 0xfffd);
    } else if (code_point < 0x1'0000) {
        r += static_cast<CharT>((code_point >> 12) | 0xe0);
        r += static_cast<CharT>(((code_point >> 6) & 0x3f) | 0x80);
        r += static_cast<CharT>((code_point & 0x3f) | 0x80);
    } else if (code_point < 0x11'0000) {
        r += static_cast<CharT>((code_point >> 18) | 0xf0);
        r += static_cast<CharT>(((code_point >> 12) & 0x3f) | 0x80);
        r += static_cast<CharT>(((code_point >> 6) & 0x3f) | 0x80);
        r += static_cast<CharT>((code_point & 0x3f) | 0x80);
    } else {
        append_code_point(r, 0xfffd);
    }
}

template<typename CharT>
constexpr void append_code_point(std::basic_string<CharT> &r, uint32_t code_point) noexcept requires(sizeof(CharT) == 2)
{
    if (code_point < 0xd800) {
        r += static_cast<CharT>(code_point);
    } else if (code_point < 0xe000) {
        append_code_point(r, 0xfffd);
    } else if (code_point < 0x1'0000) {
        r += static_cast<CharT>(code_point);
    } else if (code_point < 0x11'0000) {
        code_point -= 0x1'0000;
        r += static_cast<CharT>(0xd800 + (code_point >> 10));
        r += static_cast<CharT>(0xdc00 + (code_point & 0x3ff));
    } else {
        append_code_point(r, 0xfffd);
    }
}

template<typename CharT>
constexpr void append_code_point(std::basic_string<CharT> &r, uint32_t code_point) noexcept requires(sizeof(CharT) == 4)
{
    if (code_point < 0xd800) {
        r += static_cast<char32_t>(code_point);
    } else if (code_point < 0xe000) {
        append_code_point(r, 0xfffd);
    } else if (code_point < 0x11'0000) {
        r += static_cast<char32_t>(code_point);
    } else {
        append_code_point(r, 0xfffd);
    }
}

template<typename ToCharT, typename FromCharT>
[[nodiscard]] constexpr std::size_t guess_num_code_units(std::basic_string_view<FromCharT> const &rhs) noexcept
{
    return size(rhs);
}

template<typename ToCharT, typename FromCharT>
[[nodiscard]] constexpr std::size_t guess_num_code_units(std::basic_string_view<FromCharT> const &rhs) noexcept
    requires(sizeof(FromCharT) == 1 and sizeof(ToCharT) > 1)
{
    std::size_t r = 0;
    for (ttlet c : rhs) {
        if ((c & 0xc0) != 0x80) {
            ++r;
        }
    }
    return r;
}

template<typename ToCharT, typename FromCharT>
[[nodiscard]] constexpr std::size_t guess_num_code_units(std::basic_string_view<FromCharT> const &rhs) noexcept
    requires(sizeof(FromCharT) > 1 and sizeof(ToCharT) == 1)
{
    std::size_t r = 0;
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

template<typename ToCharT, typename FromCharT>
[[nodiscard]] constexpr std::basic_string<ToCharT> from_utf8(std::basic_string_view<FromCharT> rhs) noexcept
{
    static_assert(sizeof(FromCharT) == 1, "Expect UTF-8 string to be in a 8-bit char type.");

    auto r = std::basic_string<ToCharT>{};
    r.reserve(guess_num_code_units<ToCharT>(rhs));

    auto it = begin(rhs);
    ttlet last = end(rhs);

    uint32_t code_point = 0;
    int todo = 0;
    int num = 0;
    while (it != last) {
        ttlet c = static_cast<uint8_t>(*it);

        if (todo == 0) {
            ++it;

            if (c < 0x80) {
                todo = num = 1;
                code_point = c;

            } else if (c < 0xc0) {
                // Invalid continuation character.
                todo = 1;
                num = 3;
                code_point = 0xfffd;

            } else if (c < 0xe0) {
                todo = num = 2;
                code_point = c & 0x1f;

            } else if (c < 0xf0) {
                todo = num = 3;
                code_point = c & 0x0f;

            } else if (c < 0xf8) {
                todo = num = 4;
                code_point = c & 0x07;

            } else {
                // Invalid code-unit.
                todo = 1;
                num = 3;
                code_point = 0xfffd;
            }

        } else if ((c & 0xc0) == 0x80) {
            ++it;
            code_point <<= 6;
            code_point |= c & 0x3f;

        } else {
            // Unexpected non-continuation characters. Redo the current character as a start character.
            todo = 1;
            num = 3;
            code_point = 0xfffd;
        }

        if (--todo == 0) {
            if ((num == 2 and code_point < 0x80) or (num == 3 and code_point < 0x800) or (num == 4 and code_point < 0x1'0000)) {
                // Overlong encoding.
                code_point = 0xfffd;
            }

            append_code_point(r, code_point);
        }
    }

    if (todo) {
        append_code_point(r, 0xfffd);
    }

    return r;
}

template<typename ToCharT, typename FromCharT>
[[nodiscard]] constexpr std::basic_string<ToCharT> from_utf16(std::basic_string_view<FromCharT> rhs) noexcept
{
    static_assert(sizeof(FromCharT) == 2, "Expect UTF-16 string to be in a 16-bit char type.");

    auto r = std::basic_string<ToCharT>{};
    r.reserve(guess_num_code_units<ToCharT>(rhs));

    auto it = begin(rhs);
    ttlet last = end(rhs);

    uint32_t code_point = 0;
    int todo = 0;
    while (it != last) {
        ttlet c = *it;

        if (todo == 0) {
            ++it;
            if (c < 0xd800) {
                todo = 1;
                code_point = c;

            } else if (c < 0xdc00) {
                todo = 2;
                code_point = (c - 0xd800) << 10;

            } else if (c < 0xe000) {
                // Invalid low surrogate.
                todo = 1;
                code_point = 0xfffd;

            } else {
                todo = 1;
                code_point = c;
            }

        } else if (c >= 0xdc00 and c < 0xe000) {
            ++it;
            code_point |= (c - 0xdc00);
            code_point += 0x1'0000;

        } else {
            // Missing low surrogate, redo the current code-unit.
            todo = 1;
            code_point = 0xfffd;
        }

        if (--todo == 0) {
            append_code_point(r, code_point);
        }
    }

    if (todo) {
        append_code_point(r, 0xfffd);
    }

    return r;
}

template<typename ToCharT, typename FromCharT>
[[nodiscard]] constexpr std::basic_string<ToCharT> from_utf32(std::basic_string_view<FromCharT> rhs) noexcept
{
    static_assert(sizeof(FromCharT) == 4, "Expect UTF-32 string to be in a 32-bit char type.");

    auto r = std::basic_string<ToCharT>{};
    r.reserve(guess_num_code_units<ToCharT>(rhs));

    for (ttlet c : rhs) {
        append_code_point(r, static_cast<uint32_t>(c));
    }

    return r;
}

} // namespace detail

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

template<typename FromChar>
[[nodiscard]] constexpr std::string to_string(std::basic_string_view<FromChar> rhs) noexcept
{
    tt_static_not_implemented();
}

template<std::same_as<char16_t> FromChar>
[[nodiscard]] constexpr std::string to_string(std::basic_string_view<FromChar> rhs) noexcept
{
    return detail::from_utf16<char>(rhs);
}

template<std::same_as<char32_t> FromChar>
[[nodiscard]] constexpr std::string to_string(std::basic_string_view<FromChar> rhs) noexcept
{
    return detail::from_utf32<char>(rhs);
}

template<std::same_as<wchar_t> FromChar>
[[nodiscard]] constexpr std::string to_string(std::basic_string_view<FromChar> rhs) noexcept
    requires(sizeof(FromChar) == sizeof(char16_t))
{
    return detail::from_utf16<char>(rhs);
}

template<std::same_as<wchar_t> FromChar>
[[nodiscard]] constexpr std::string to_string(std::basic_string_view<FromChar> rhs) noexcept
    requires(sizeof(FromChar) == sizeof(char32_t))
{
    return detail::from_utf32<char>(rhs);
}

template<typename FromChar>
[[nodiscard]] constexpr std::u16string to_u16string(std::basic_string_view<FromChar> rhs) noexcept
{
    tt_static_not_implemented();
}

template<std::same_as<char> FromChar>
[[nodiscard]] constexpr std::u16string to_u16string(std::basic_string_view<FromChar> rhs) noexcept
{
    return detail::from_utf8<char16_t>(rhs);
}

template<std::same_as<char32_t> FromChar>
[[nodiscard]] constexpr std::u16string to_u16string(std::basic_string_view<FromChar> rhs) noexcept
{
    return detail::from_utf32<char16_t>(rhs);
}

template<std::same_as<wchar_t> FromChar>
[[nodiscard]] constexpr std::u16string to_u16string(std::basic_string_view<FromChar> rhs) noexcept
    requires(sizeof(FromChar) == sizeof(char16_t))
{
    return detail::from_utf16<char16_t>(rhs);
}

template<std::same_as<wchar_t> FromChar>
[[nodiscard]] constexpr std::u16string to_u16string(std::basic_string_view<FromChar> rhs) noexcept
    requires(sizeof(FromChar) == sizeof(char32_t))
{
    return detail::from_utf32<char16_t>(rhs);
}

template<typename FromChar>
[[nodiscard]] constexpr std::u32string to_u32string(std::basic_string_view<FromChar> rhs) noexcept
{
    tt_static_not_implemented();
}

template<std::same_as<char> FromChar>
[[nodiscard]] constexpr std::u32string to_u32string(std::basic_string_view<FromChar> rhs) noexcept
{
    return detail::from_utf8<char32_t>(rhs);
}

template<std::same_as<char16_t> FromChar>
[[nodiscard]] constexpr std::u32string to_u32string(std::basic_string_view<FromChar> rhs) noexcept
{
    return detail::from_utf16<char32_t>(rhs);
}

template<std::same_as<wchar_t> FromChar>
[[nodiscard]] constexpr std::u32string to_u32string(std::basic_string_view<FromChar> rhs) noexcept
    requires(sizeof(FromChar) == sizeof(char16_t))
{
    return detail::from_utf16<char32_t>(rhs);
}

template<std::same_as<wchar_t> FromChar>
[[nodiscard]] constexpr std::u32string to_u32string(std::basic_string_view<FromChar> rhs) noexcept
    requires(sizeof(FromChar) == sizeof(char32_t))
{
    return detail::from_utf32<char32_t>(rhs);
}

template<typename FromChar>
[[nodiscard]] constexpr std::wstring to_wstring(std::basic_string_view<FromChar> rhs) noexcept
{
    tt_static_not_implemented();
}

template<std::same_as<char> FromChar>
[[nodiscard]] constexpr std::wstring to_wstring(std::basic_string_view<FromChar> rhs) noexcept
{
    return detail::from_utf8<wchar_t>(rhs);
}

template<std::same_as<char16_t> FromChar>
[[nodiscard]] constexpr std::wstring to_wstring(std::basic_string_view<FromChar> rhs) noexcept
{
    return detail::from_utf16<wchar_t>(rhs);
}

template<std::same_as<char32_t> FromChar>
[[nodiscard]] constexpr std::wstring to_wstring(std::basic_string_view<FromChar> rhs) noexcept
{
    return detail::from_utf32<wchar_t>(rhs);
}

template<typename FromChar>
[[nodiscard]] constexpr std::string to_string(std::basic_string<FromChar> const &rhs) noexcept
{
    return to_string(std::basic_string_view<FromChar>(rhs));
}

template<typename FromChar>
[[nodiscard]] constexpr std::u16string to_u16string(std::basic_string<FromChar> const &rhs) noexcept
{
    return to_u16string(std::basic_string_view<FromChar>(rhs));
}

template<typename FromChar>
[[nodiscard]] constexpr std::u32string to_u32string(std::basic_string<FromChar> const &rhs) noexcept
{
    return to_u32string(std::basic_string_view<FromChar>(rhs));
}

template<typename FromChar>
[[nodiscard]] std::wstring to_wstring(std::basic_string<FromChar> const &rhs) noexcept
{
    return to_wstring(std::basic_string_view<FromChar>(rhs));
}

template<typename FromChar>
[[nodiscard]] std::string to_string(FromChar const *rhs) noexcept
{
    return to_string(std::basic_string_view<FromChar>(rhs));
}

template<typename FromChar>
[[nodiscard]] std::u16string to_u16string(FromChar const *rhs) noexcept
{
    return to_u16string(std::basic_string_view<FromChar>(rhs));
}

template<typename FromChar>
[[nodiscard]] std::u32string to_u32string(FromChar const *rhs) noexcept
{
    return to_u32string(std::basic_string_view<FromChar>(rhs));
}

template<typename FromChar>
[[nodiscard]] std::wstring to_wstring(FromChar const *rhs) noexcept
{
    return to_wstring(std::basic_string_view<FromChar>(rhs));
}

} // namespace tt::inline v1
