// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "UTF.hpp"
#include <string>
#include <string_view>
#include <bit>
#include <type_traits>

namespace tt {
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
[[nodiscard]] constexpr size_t guess_num_code_units(std::basic_string_view<FromCharT> const &rhs) noexcept
{
    return std::size(rhs);
}

template<typename ToCharT, typename FromCharT>
[[nodiscard]] constexpr size_t guess_num_code_units(std::basic_string_view<FromCharT> const &rhs) noexcept
    requires(sizeof(FromCharT) == 1 and sizeof(ToCharT) > 1)
{
    size_t r = 0;
    for (ttlet c : rhs) {
        if ((c & 0xc0) != 0x80) {
            ++r;
        }
    }
    return r;
}

template<typename ToCharT, typename FromCharT>
[[nodiscard]] constexpr size_t guess_num_code_units(std::basic_string_view<FromCharT> const &rhs) noexcept
    requires(sizeof(FromCharT) > 1 and sizeof(ToCharT) == 1)
{
    size_t r = 0;
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

    auto it = std::begin(rhs);
    ttlet last = std::end(rhs);

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

    auto it = std::begin(rhs);
    ttlet last = std::end(rhs);

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

[[nodiscard]] std::u32string utf8_to_utf32(std::string_view rhs) noexcept
{
    return detail::from_utf8<char32_t>(rhs);
}

[[nodiscard]] std::u16string utf8_to_utf16(std::string_view rhs) noexcept
{
    return detail::from_utf8<char16_t>(rhs);
}

[[nodiscard]] std::string utf8_to_utf8(std::string_view rhs) noexcept
{
    return detail::from_utf8<char>(rhs);
}

[[nodiscard]] std::wstring utf8_to_wide(std::string_view rhs) noexcept
{
    return detail::from_utf8<wchar_t>(rhs);
}

[[nodiscard]] std::u32string utf16_to_utf32(std::u16string_view rhs) noexcept
{
    return detail::from_utf16<char32_t>(rhs);
}

[[nodiscard]] std::u16string utf16_to_utf16(std::u16string_view rhs) noexcept
{
    return detail::from_utf16<char16_t>(rhs);
}

[[nodiscard]] std::string utf16_to_utf8(std::u16string_view rhs) noexcept
{
    return detail::from_utf16<char>(rhs);
}

[[nodiscard]] std::wstring utf16_to_wide(std::u16string_view rhs) noexcept
{
    return detail::from_utf16<wchar_t>(rhs);
}

[[nodiscard]] std::u32string utf32_to_utf32(std::u32string_view rhs) noexcept
{
    return detail::from_utf32<char32_t>(rhs);
}

[[nodiscard]] std::u16string utf32_to_utf16(std::u32string_view rhs) noexcept
{
    return detail::from_utf32<char16_t>(rhs);
}

[[nodiscard]] std::string utf32_to_utf8(std::u32string_view rhs) noexcept
{
    return detail::from_utf32<char>(rhs);
}

[[nodiscard]] std::wstring utf32_to_wide(std::u32string_view rhs) noexcept
{
    return detail::from_utf32<wchar_t>(rhs);
}

[[nodiscard]] std::u32string wide_to_utf32(std::wstring_view rhs) noexcept
{
    if constexpr (sizeof(wchar_t) == 4) {
        return detail::from_utf32<char32_t>(rhs);
    } else {
        return detail::from_utf16<char32_t>(rhs);
    }
}

[[nodiscard]] std::u16string wide_to_utf16(std::wstring_view rhs) noexcept
{
    if constexpr (sizeof(wchar_t) == 4) {
        return detail::from_utf32<char16_t>(rhs);
    } else {
        return detail::from_utf16<char16_t>(rhs);
    }
}

[[nodiscard]] std::string wide_to_utf8(std::wstring_view rhs) noexcept
{
    if constexpr (sizeof(wchar_t) == 4) {
        return detail::from_utf32<char>(rhs);
    } else {
        return detail::from_utf16<char>(rhs);
    }
}

[[nodiscard]] std::wstring wide_to_wide(std::wstring_view rhs) noexcept
{
    if constexpr (sizeof(wchar_t) == 4) {
        return detail::from_utf32<wchar_t>(rhs);
    } else {
        return detail::from_utf16<wchar_t>(rhs);
    }
}

[[nodiscard]] std::string to_string(std::u16string_view rhs) noexcept
{
    return utf16_to_utf8(rhs);
}

[[nodiscard]] std::string to_string(std::u32string_view rhs) noexcept
{
    return utf32_to_utf8(rhs);
}

[[nodiscard]] std::string to_string(std::wstring_view rhs) noexcept
{
    return wide_to_utf8(rhs);
}

[[nodiscard]] std::u16string to_u16string(std::string_view rhs) noexcept
{
    return utf8_to_utf16(rhs);
}

[[nodiscard]] std::u16string to_u16string(std::wstring_view rhs) noexcept
{
    return wide_to_utf16(rhs);
}

[[nodiscard]] std::u16string to_u16string(std::u32string_view rhs) noexcept
{
    return utf32_to_utf16(rhs);
}

[[nodiscard]] std::u32string to_u32string(std::string_view rhs) noexcept
{
    return utf8_to_utf32(rhs);
}

[[nodiscard]] std::u32string to_u32string(std::u16string_view rhs) noexcept
{
    return utf16_to_utf32(rhs);
}

[[nodiscard]] std::u32string to_u32string(std::wstring_view rhs) noexcept
{
    return wide_to_utf32(rhs);
}

[[nodiscard]] std::wstring to_wstring(std::string_view rhs) noexcept
{
    return utf8_to_wide(rhs);
}

[[nodiscard]] std::wstring to_wstring(std::u16string_view rhs) noexcept
{
    return utf16_to_wide(rhs);
}

[[nodiscard]] std::wstring to_wstring(std::u32string_view rhs) noexcept
{
    return utf32_to_wide(rhs);
}

} // namespace tt
