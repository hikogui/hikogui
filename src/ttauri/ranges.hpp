// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "coroutine.hpp"
#include "type_traits.hpp"
#include <ranges>
#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>

namespace tt::ranges::views {

template<
    std::ranges::range Haystack,
    std::ranges::range Needle,
    typename SubHaystack = std::conditional_t<
        is_character_v<std::ranges::range_value_t<Haystack>>,
        make_string_view_t<std::ranges::range_value_t<Haystack>>,
        std::ranges::subrange<std::ranges::iterator_t<Haystack>, std::ranges::iterator_t<Haystack>>>>
[[nodiscard]] generator<SubHaystack> split(Haystack &haystack, Needle needle) noexcept
{
    auto it = std::begin(haystack);
    auto last = std::end(haystack);

    auto needle_first = std::ranges::begin(needle);
    auto needle_last = std::ranges::end(needle);
    auto needle_length = std::distance(needle_first, needle_last);

    while (it != last) {
        auto sep_start = std::search(it, last, needle_first, needle_last);
        if (sep_start != last) {
            co_yield SubHaystack{it, sep_start};
            it = sep_start + needle_length;
        } else {
            it = sep_start;
        }
    }
}

[[nodiscard]] inline generator<std::string_view> split(std::string_view haystack, char const *needle = " ") noexcept
{
    return split(haystack, std::string{needle});
}

[[nodiscard]] inline generator<std::string_view> split(std::string &haystack, char const *needle = " ") noexcept
{
    return split(haystack, std::string{needle});
}

[[nodiscard]] inline generator<std::wstring_view> split(std::wstring_view haystack, wchar_t const *needle = L" ") noexcept
{
    return split(haystack, std::wstring{needle});
}

[[nodiscard]] inline generator<std::wstring_view> split(std::wstring &haystack, wchar_t const *needle = L" ") noexcept
{
    return split(haystack, std::wstring{needle});
}

[[nodiscard]] inline generator<std::u8string_view> split(std::u8string_view haystack, char8_t const *needle = u8" ") noexcept
{
    return split(haystack, std::u8string{needle});
}

[[nodiscard]] inline generator<std::u8string_view> split(std::u8string &haystack, char8_t const *needle = u8" ") noexcept
{
    return split(haystack, std::u8string{needle});
}

[[nodiscard]] inline generator<std::u16string_view> split(std::u16string_view haystack, char16_t const *needle = u" ") noexcept
{
    return split(haystack, std::u16string{needle});
}

[[nodiscard]] inline generator<std::u16string_view> split(std::u16string &haystack, char16_t const *needle = u" ") noexcept
{
    return split(haystack, std::u16string{needle});
}

[[nodiscard]] inline generator<std::u32string_view> split(std::u32string_view haystack, char32_t const *needle = U" ") noexcept
{
    return split(haystack, std::u32string{needle});
}

[[nodiscard]] inline generator<std::u32string_view> split(std::u32string &haystack, char32_t const *needle = U" ") noexcept
{
    return split(haystack, std::u32string{needle});
}

} // namespace tt::ranges::views

namespace tt {
namespace views = tt::ranges::views;
}