


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
    using char_type = CharT;

    char_type _str[N + 1];

    constexpr basic_fixed_string(basic_fixed_string const &) noexcept = default;
    constexpr basic_fixed_string(basic_fixed_string &&) noexcept = default;
    constexpr basic_fixed_string &operator=(basic_fixed_string const &) noexcept = default;
    constexpr basic_fixed_string &operator=(basic_fixed_string &&) noexcept = default;

    [[nodiscard]] constexpr basic_fixed_string() noexcept
    {
        std::memset(&_str[0], 0, sizeof(CharT) * N);
    }

    [[nodiscard]] constexpr basic_fixed_string(char_type const (&str)[N + 1]) noexcept
    {
        std::copy_n(str, N + 1, _str);
    }

    [[nodiscard]] operator std::string () const noexcept
    {
        return std::string{data(), size()};
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

    [[nodiscard]] constexpr char_type const *data() const noexcept
    {
        return &_str[0];
    }

    template<int M>
    [[nodiscard]] constexpr auto operator+(basic_fixed_string<char_type, M> const &rhs) noexcept
    {
        auto r = basic_fixed_string<char_type, N+M>{};
        std::copy_n(data(), N, &r._str[0]);
        std::copy_n(_rhs.data(), M + 1, &r._str[N]);
        return r;
    }

    [[nodiscard]] bool operator==(basic_fixed_string const &lhs, basic_fixed_string const &rhs) noexcept = default;
};

template<typename CharT, size_t N>
basic_fixed_string(CharT (&)[N]) -> basic_fixed_string<CharT, N - 1>;


template<int N>
using fixed_string = basic_fixed_string<char,N>;

template<int N>
using fixed_wstring = basic_fixed_string<wchar_t,N>;

template<int N>
using fixed_u8string = basic_fixed_string<char8_t,N>;

template<int N>
using fixed_u16string = basic_fixed_string<char16_t,N>;

template<int N>
using fixed_u32string = basic_fixed_string<char32_t,N>;


}

