// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include <string>
#include <string_view>
#include <format>
#include <array>

namespace hi::inline v1 {

/**
 *
 * example:
 *     ```
 *     template<hi::basic_fixed_string Foo>
 *     class A {
 *         auto bar() {
 *             return std::string{Foo};
 *         }
 *     };
 *
 *     std::string test() {
 *         auto a = A<"hello">{};
 *         return a.bar();
 *     }
 *     ```
 */
template<typename CharT, int N>
struct basic_fixed_string {
    using value_type = CharT;

    std::array<value_type, N> _str;

    constexpr basic_fixed_string() noexcept : _str{} {}

    constexpr basic_fixed_string(basic_fixed_string const&) noexcept = default;
    constexpr basic_fixed_string(basic_fixed_string&&) noexcept = default;
    constexpr basic_fixed_string& operator=(basic_fixed_string const&) noexcept = default;
    constexpr basic_fixed_string& operator=(basic_fixed_string&&) noexcept = default;

    template<std::size_t O>
    constexpr basic_fixed_string(value_type const (&str)[O]) noexcept requires((O - 1) == N) : _str{}
    {
        for (auto i = 0_uz; i != (O - 1); ++i) {
            _str[i] = str[i];
        }
    }

    template<std::size_t O>
    constexpr basic_fixed_string& operator=(value_type const (&str)[O]) noexcept requires((O - 1) == N)
    {
        auto i = 0_uz;
        for (; i != (O - 1); ++i) {
            _str[i] = str[i];
        }
        for (; i != N; ++i) {
            _str[i] = value_type{};
        }
        return *this;
    }

    constexpr operator std::basic_string_view<value_type>() const noexcept
    {
        return std::basic_string_view<value_type>{_str.data(), size()};
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return N;
    }

    [[nodiscard]] constexpr auto begin() noexcept
    {
        return _str.begin();
    }

    [[nodiscard]] constexpr auto end() noexcept
    {
        return _str.begin() + size();
    }

    [[nodiscard]] constexpr bool operator==(basic_fixed_string const& rhs) const noexcept = default;
    [[nodiscard]] constexpr auto operator<=>(basic_fixed_string const& rhs) const noexcept = default;

    template<size_t O>
    [[nodiscard]] constexpr bool operator==(basic_fixed_string<CharT, O> const& rhs) const noexcept requires(O != N)
    {
        return false;
    }

    template<size_t O>
    [[nodiscard]] constexpr auto operator<=>(basic_fixed_string<CharT, O> const& rhs) const noexcept requires(O != N)
    {
        return static_cast<std::basic_string_view<CharT>>(*this) <=> static_cast<std::basic_string_view<CharT>>(rhs);
    }

    [[nodiscard]] constexpr bool operator==(std::basic_string_view<CharT> rhs) const noexcept
    {
        return static_cast<std::basic_string_view<CharT>>(*this) == rhs;
    }

    [[nodiscard]] constexpr auto operator<=>(std::basic_string_view<CharT> rhs) const noexcept
    {
        return static_cast<std::basic_string_view<CharT>>(*this) <=> rhs;
    }
};

template<std::size_t N>
using fixed_string = basic_fixed_string<char, N>;

// template<typename CharT>
//[[nodiscard]] constexpr std::size_t basic_fixed_string_length_(CharT const *str) noexcept
//{
//     std::size_t i = 0;
//     while (str[i++] != CharT{}) {}
//     return i;
// }

template<typename CharT, std::size_t N>
basic_fixed_string(CharT const (&str)[N]) -> basic_fixed_string<CharT, N - 1>;

} // namespace hi::inline v1

template<typename T, std::size_t N, typename CharT>
struct std::formatter<hi::basic_fixed_string<T, N>, CharT> : std::formatter<std::basic_string_view<T>, CharT> {
    auto format(hi::basic_fixed_string<T, N> const& t, auto& fc)
    {
        return std::formatter<std::basic_string_view<T>, CharT>::format(static_cast<std::basic_string_view<T>>(t), fc);
    }
};
