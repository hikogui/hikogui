// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utf_8.hpp"
#include "utf_16.hpp"
#include "utf_32.hpp"
#include <string>
#include <string_view>
#include <climits>

namespace hi::inline v1 {

[[nodiscard]] constexpr std::u32string to_u32string(std::u32string_view rhs) noexcept
{
    return char_converter<"utf-32", "utf-32">{}(rhs);
}

[[nodiscard]] constexpr std::u32string to_u32string(std::u16string_view rhs) noexcept
{
    return char_converter<"utf-16", "utf-32">{}(rhs);
}

[[nodiscard]] constexpr std::u32string to_u32string(std::u8string_view rhs) noexcept
{
    return char_converter<"utf-8", "utf-32">{}(rhs);
}

[[nodiscard]] constexpr std::u32string to_u32string(std::wstring_view rhs) noexcept
{
#if WCHAR_MAX >= 0x10'ffff
    return char_converter<"utf-32", "utf-32">{}(rhs);
#else
    return char_converter<"utf-16", "utf-32">{}(rhs);
#endif
}

[[nodiscard]] constexpr std::u32string to_u32string(std::string_view rhs) noexcept
{
    return char_converter<"utf-8", "utf-32">{}(rhs);
}

[[nodiscard]] constexpr std::u16string to_u16string(std::u32string_view rhs) noexcept
{
    return char_converter<"utf-32", "utf-16">{}(rhs);
}

[[nodiscard]] constexpr std::u16string to_u16string(std::u16string_view rhs) noexcept
{
    return char_converter<"utf-16", "utf-16">{}(rhs);
}

[[nodiscard]] constexpr std::u16string to_u16string(std::u8string_view rhs) noexcept
{
    return char_converter<"utf-8", "utf-16">{}(rhs);
}

[[nodiscard]] constexpr std::u16string to_u16string(std::wstring_view rhs) noexcept
{
#if WCHAR_MAX >= 0x10'ffff
    return char_converter<"utf-32", "utf-16">{}(rhs);
#else
    return char_converter<"utf-16", "utf-16">{}(rhs);
#endif
}

[[nodiscard]] constexpr std::u16string to_u16string(std::string_view rhs) noexcept
{
    return char_converter<"utf-8", "utf-16">{}(rhs);
}

[[nodiscard]] constexpr std::u8string to_u8string(std::u32string_view rhs) noexcept
{
    return char_converter<"utf-32", "utf-8">{}.convert<std::u8string>(rhs);
}

[[nodiscard]] constexpr std::u8string to_u8string(std::u16string_view rhs) noexcept
{
    return char_converter<"utf-16", "utf-8">{}.convert<std::u8string>(rhs);
}

[[nodiscard]] constexpr std::u8string to_u8string(std::u8string_view rhs) noexcept
{
    return char_converter<"utf-8", "utf-8">{}.convert<std::u8string>(rhs);
}

[[nodiscard]] constexpr std::u8string to_u8string(std::wstring_view rhs) noexcept
{
#if WCHAR_MAX >= 0x10'ffff
    return char_converter<"utf-32", "utf-8">{}.convert<std::u8string>(rhs);
#else
    return char_converter<"utf-16", "utf-8">{}.convert<std::u8string>(rhs);
#endif
}

[[nodiscard]] constexpr std::u8string to_u8string(std::string_view rhs) noexcept
{
    return char_converter<"utf-8", "utf-8">{}.convert<std::u8string>(rhs);
}

[[nodiscard]] constexpr std::wstring to_wstring(std::u32string_view rhs) noexcept
{
#if WCHAR_MAX >= 0x10'ffff
    return char_converter<"utf-32", "utf-32">{}.convert<std::wstring>(rhs);
#else
    return char_converter<"utf-32", "utf-16">{}.convert<std::wstring>(rhs);
#endif
}

[[nodiscard]] constexpr std::wstring to_wstring(std::u16string_view rhs) noexcept
{
#if WCHAR_MAX >= 0x10'ffff
    return char_converter<"utf-16", "utf-32">{}.convert<std::wstring>(rhs);
#else
    return char_converter<"utf-16", "utf-16">{}.convert<std::wstring>(rhs);
#endif
}

[[nodiscard]] constexpr std::wstring to_wstring(std::u8string_view rhs) noexcept
{
#if WCHAR_MAX >= 0x10'ffff
    return char_converter<"utf-8", "utf-32">{}.convert<std::wstring>(rhs);
#else
    return char_converter<"utf-8", "utf-16">{}.convert<std::wstring>(rhs);
#endif
}

[[nodiscard]] constexpr std::wstring to_wstring(std::wstring_view rhs) noexcept
{
#if WCHAR_MAX >= 0x10'ffff
    return char_converter<"utf-32", "utf-32">{}.convert<std::wstring>(rhs);
#else
    return char_converter<"utf-16", "utf-16">{}.convert<std::wstring>(rhs);
#endif
}

[[nodiscard]] constexpr std::wstring to_wstring(std::string_view rhs) noexcept
{
#if WCHAR_MAX >= 0x10'ffff
    return char_converter<"utf-8", "utf-32">{}.convert<std::wstring>(rhs);
#else
    return char_converter<"utf-8", "utf-16">{}.convert<std::wstring>(rhs);
#endif
}

[[nodiscard]] constexpr std::string to_string(std::u32string_view rhs) noexcept
{
    return char_converter<"utf-32", "utf-8">{}(rhs);
}

[[nodiscard]] constexpr std::string to_string(std::u16string_view rhs) noexcept
{
    return char_converter<"utf-16", "utf-8">{}(rhs);
}

[[nodiscard]] constexpr std::string to_string(std::u8string_view rhs) noexcept
{
    return char_converter<"utf-8", "utf-8">{}(rhs);
}

[[nodiscard]] constexpr std::string to_string(std::wstring_view rhs) noexcept
{
#if WCHAR_MAX >= 0x10'ffff
    return char_converter<"utf-32", "utf-8">{}(rhs);
#else
    return char_converter<"utf-16", "utf-8">{}(rhs);
#endif
}

[[nodiscard]] constexpr std::string to_string(std::string_view rhs) noexcept
{
    return char_converter<"utf-8", "utf-8">{}(rhs);
}

} // namespace hi::inline v1
