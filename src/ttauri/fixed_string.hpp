// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "strings.hpp"
#include <string>
#include <string_view>
#include <format>
#include <array>

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

    std::array<value_type, N> _str;

    constexpr basic_fixed_string() noexcept : _str{}
    {
    }

    constexpr basic_fixed_string(basic_fixed_string const &) noexcept = default;
    constexpr basic_fixed_string(basic_fixed_string &&) noexcept = default;
    constexpr basic_fixed_string &operator=(basic_fixed_string const &) noexcept = default;
    constexpr basic_fixed_string &operator=(basic_fixed_string &&) noexcept = default;

    template<std::size_t O>
    constexpr basic_fixed_string(basic_fixed_string<value_type, O> const &other) noexcept requires(O < N) : _str{}
    {
        for (auto i = 0_uz; i != O; ++i) {
            _str[i] = other._str[i];
        }
    }

    template<std::size_t O>
    constexpr basic_fixed_string &operator=(basic_fixed_string<value_type, O> const &other) noexcept requires(O < N)
    {
        auto i = 0_uz;
        for (; i != O; ++i) {
            _str[i] = other._str[i];
        }
        for (; i != N; ++i) {
            _str[i] = value_type{};
        }
        return *this;
    }

    template<std::size_t O>
    constexpr basic_fixed_string(value_type const (&str)[O]) noexcept : _str{}
    {
        static_assert((O - 1) <= N);

        for (auto i = 0_uz; i != (O - 1); ++i) {
            _str[i] = str[i];
        }
    }

    template<std::size_t O>
    constexpr basic_fixed_string &operator=(value_type const (&str)[O]) noexcept
    {
        static_assert((O - 1) <= N);

        auto i = 0_uz;
        for (; i != (O - 1); ++i) {
            _str[i] = str[i];
        }
        for (; i != N; ++i) {
            _str[i] = value_type{};
        }
        return *this;
    }

    constexpr explicit basic_fixed_string(std::basic_string_view<value_type> str) noexcept : _str{}
    {
        tt_axiom(str.size() <= N);

        for (auto i = 0_uz; i != str.size(); ++i) {
            _str[i] = str[i];
        }
    }

    constexpr explicit basic_fixed_string(std::basic_string<value_type> const &str) noexcept : _str{}
    {
        tt_axiom(str.size() <= N);

        for (auto i = 0_uz; i != str.size(); ++i) {
            _str[i] = str[i];
        }
    }

    /** Initialize the string from a nul-terminated c-string.
     */
    constexpr explicit basic_fixed_string(value_type const *str) noexcept : _str{}
    {
        auto i = 0_uz;
        for (; i != N and str[i] != value_type{}; ++i) {
            _str[i] = str[i];
        }
        
        tt_axiom(str[i] == value_type{});
    }

    operator std::basic_string_view<value_type>() const noexcept
    {
        return std::basic_string_view<value_type>{_str.data(), size()};
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        for (auto i = 0_uz; i != N; ++i) {
            if (_str[i] == value_type{}) {
                return i;
            }
        }
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

    /** Convert the current string to using title case.
    * 
    * This function does not do full unicode case conversion;
    * only ASCII letters [a-zA-Z] will be modified.
    */
    [[nodiscard]] friend constexpr basic_fixed_string to_title(basic_fixed_string const &rhs) noexcept
    {
        auto r = rhs;

        bool first = true;
        for (auto &c: r) {
            if (first) {
                c = to_upper(c);
                first = false;
            } else if (c == ' ') {
                first = true;
            } else {
                c = to_lower(c);
            }
        }

        return r;
    }

    [[nodiscard]] friend constexpr bool
    operator==(basic_fixed_string const &lhs, basic_fixed_string const &rhs) noexcept = default;
    [[nodiscard]] friend constexpr auto
    operator<=>(basic_fixed_string const &lhs, basic_fixed_string const &rhs) noexcept = default;

    [[nodiscard]] friend constexpr bool
    operator==(std::basic_string_view<value_type> const &lhs, basic_fixed_string const &rhs) noexcept
    {
        return lhs == static_cast<decltype(lhs)>(rhs);
    }

    [[nodiscard]] friend constexpr bool
    operator==(basic_fixed_string const &lhs, std::basic_string_view<value_type> const &rhs) noexcept
    {
        return static_cast<decltype(rhs)>(lhs) == rhs;
    }
};

template<std::size_t N>
using fixed_string = basic_fixed_string<char, N>;

//template<typename CharT>
//[[nodiscard]] constexpr std::size_t basic_fixed_string_length_(CharT const *str) noexcept
//{
//    std::size_t i = 0;
//    while (str[i++] != CharT{}) {}
//    return i;
//}


template<typename CharT, std::size_t N>
basic_fixed_string(CharT const (&str)[N]) -> basic_fixed_string<CharT, N - 1>;

} // namespace tt::inline v1

template<typename T, std::size_t N, typename CharT>
struct std::formatter<tt::basic_fixed_string<T, N>, CharT> : std::formatter<T const *, CharT> {
    auto format(tt::basic_fixed_string<T, N> const &t, auto &fc)
    {
        return std::formatter<T const *, CharT>::format(t.data(), fc);
    }
};
