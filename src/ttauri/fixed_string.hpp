// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)



#pragma once

namespace tt {

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

    value_type _str[N + 1];

    constexpr basic_fixed_string(basic_fixed_string const &) noexcept = default;
    constexpr basic_fixed_string(basic_fixed_string &&) noexcept = default;
    constexpr basic_fixed_string &operator=(basic_fixed_string const &) noexcept = default;
    constexpr basic_fixed_string &operator=(basic_fixed_string &&) noexcept = default;

    [[nodiscard]] constexpr basic_fixed_string() noexcept : _str()
    {
        std::memset(&_str[0], 0, sizeof(CharT) * N);
    }

    [[nodiscard]] constexpr basic_fixed_string(value_type const (&str)[N + 1]) noexcept : _str()
    {
        std::copy_n(str, N + 1, _str);
    }

    [[nodiscard]] operator std::basic_string<value_type>() const noexcept
    {
        return std::basic_string<value_type>{data(), size()};
    }

    [[nodiscard]] operator std::basic_string_view<value_type>() const noexcept
    {
        return std::basic_string_view<value_type>{data(), size()};
    }

    [[nodiscard]] operator value_type const *() const noexcept
    {
        return data();
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
        return N;
    }

    [[nodiscard]] constexpr value_type const *data() const noexcept
    {
        return &_str[0];
    }

    template<int M>
    [[nodiscard]] constexpr auto operator+(basic_fixed_string<value_type, M> const &rhs) noexcept
    {
        auto r = basic_fixed_string<value_type, N+M>{};
        std::copy_n(data(), N, &r._str[0]);
        std::copy_n(rhs.data(), M + 1, &r._str[N]);
        return r;
    }

    [[nodiscard]] friend bool operator==(basic_fixed_string const &lhs, basic_fixed_string const &rhs) noexcept = default;
};

template<typename CharT, size_t N>
basic_fixed_string(CharT const (&)[N]) -> basic_fixed_string<CharT, N - 1>;


template<size_t N>
using fixed_string = basic_fixed_string<char,N>;

template<size_t N>
using fixed_wstring = basic_fixed_string<wchar_t,N>;

template<size_t N>
using fixed_u8string = basic_fixed_string<char8_t,N>;

template<size_t N>
using fixed_u16string = basic_fixed_string<char16_t,N>;

template<size_t N>
using fixed_u32string = basic_fixed_string<char32_t,N>;


}

