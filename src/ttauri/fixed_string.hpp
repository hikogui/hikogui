// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <string_view>
#include <format>

namespace tt::inline v1 {

/**
 *
 * example:
 *     ```
 *     template<tt::basic_fixed_string Foo>
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

    CharT _str[N];

    constexpr basic_fixed_string(basic_fixed_string const &) noexcept = default;
    constexpr basic_fixed_string(basic_fixed_string &&) noexcept = default;
    constexpr basic_fixed_string &operator=(basic_fixed_string const &) noexcept = default;
    constexpr basic_fixed_string &operator=(basic_fixed_string &&) noexcept = default;

    [[nodiscard]] constexpr basic_fixed_string() noexcept : _str()
    {
        for (size_t i = 0; i != N; ++i) {
            _str[i] = CharT{};
        }
    }

    [[nodiscard]] constexpr basic_fixed_string(CharT const (&str)[N]) noexcept : _str()
    {
        for (size_t i = 0; i != N; ++i) {
            _str[i] = str[i];
        }
    }

    [[nodiscard]] operator std::basic_string_view<CharT>() const noexcept
    {
        return std::basic_string_view<CharT>{data(), size()};
    }

    [[nodiscard]] constexpr auto begin() const noexcept
    {
        return &_str[0];
    }

    [[nodiscard]] constexpr auto end() const noexcept
    {
        return &_str[N];
    }

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return N - 1;
    }

    [[nodiscard]] constexpr friend size_t size(basic_fixed_string const &rhs) noexcept
    {
        return rhs.size();
    }

    [[nodiscard]] constexpr CharT const *data() const noexcept
    {
        return &_str[0];
    }

    [[nodiscard]] constexpr CharT const *c_str() const noexcept
    {
        return &_str[0];
    }

    [[nodiscard]] friend bool operator==(basic_fixed_string const &lhs, basic_fixed_string const &rhs) noexcept = default;
};

template<typename CharT>
[[nodiscard]] constexpr size_t basic_fixed_string_length_(CharT const *str) noexcept
{
    size_t i = 0;
    while (str[i++] != CharT{}) {
    }
    return i;
}

} // namespace tt

namespace std {

template<typename T, size_t N, typename CharT>
struct std::formatter<tt::basic_fixed_string<T, N>, CharT> : std::formatter<T const *, CharT> {
    auto format(tt::basic_fixed_string<T, N> const &t, auto &fc)
    {
        return std::formatter<T const *, CharT>::format(t.data(), fc);
    }
};

} // namespace std
