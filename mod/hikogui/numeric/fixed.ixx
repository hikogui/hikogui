// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <type_traits>
#include <limits>
#include <concepts>
#include <string>
#include <ostream>

export module hikogui_numeric_fixed;
import hikogui_numeric_safe_int;
import hikogui_utility;

export namespace hi::inline v1 {

template<typename T, int M>
struct fixed {
    using value_type = T;
    constexpr static int multiplier = M;

    T value;

    fixed() = default;
    ~fixed() = default;
    fixed(fixed const &) = default;
    fixed &operator=(fixed const &) = default;
    fixed(fixed &&) = default;
    fixed &operator=(fixed &&) = default;

    explicit constexpr fixed(std::floating_point auto other) noexcept : value(static_cast<T>(other * M))
    {
        hi_assert(other >= (std::numeric_limits<T>::min() / M) && other <= (std::numeric_limits<T>::max() / M));
    }

    explicit constexpr fixed(std::integral auto other) noexcept : value(static_cast<T>(other) * M)
    {
        hi_assert(other >= (std::numeric_limits<T>::min() / M) && other <= (std::numeric_limits<T>::max() / M));
    }

    explicit fixed(std::string const &other) : fixed(stod(other)) {}

    constexpr fixed &operator=(std::floating_point auto other) noexcept
    {
        value = static_cast<T>(other * M);
        hi_assert(other >= (std::numeric_limits<T>::min() / M) && other <= (std::numeric_limits<T>::max() / M));
        return *this;
    }

    constexpr fixed &operator=(std::integral auto other) noexcept
    {
        value = static_cast<T>(other) * M;
        hi_assert(other >= (std::numeric_limits<T>::min() / M) && other <= (std::numeric_limits<T>::max() / M));
        return *this;
    }

    fixed &operator=(std::string const &other)
    {
        value = static_cast<T>(stod(other) * M);
        hi_assert(other >= (std::numeric_limits<T>::min() / M) && other <= (std::numeric_limits<T>::max() / M));
        return *this;
    }

    template<std::floating_point O>
    explicit operator O() const noexcept
    {
        return static_cast<O>(value) / M;
    }

    template<std::integral O>
    explicit operator O() const noexcept
    {
        return static_cast<O>(value / M);
    }

    std::string string() const noexcept
    {
        return std::format("{}", static_cast<double>(value) / M);
    }

    static fixed from_raw_value(T value) noexcept
    {
        fixed r;
        r.value = value;
        return r;
    }

    [[nodiscard]] constexpr friend bool operator==(fixed const &lhs, fixed const &rhs) noexcept
    {
        return lhs.value == rhs.value;
    }

    [[nodiscard]] constexpr friend auto operator<=>(fixed const &lhs, fixed const &rhs) noexcept
    {
        return lhs.value <=> rhs.value;
    }

    [[nodiscard]] constexpr friend fixed operator+(fixed const &lhs, fixed const &rhs) noexcept
    {
        return fixed<T, M>::from_raw_value(lhs.value + rhs.value);
    }

    [[nodiscard]] constexpr friend fixed operator-(fixed const &lhs, fixed const &rhs) noexcept
    {
        return fixed<T, M>::from_raw_value(lhs.value - rhs.value);
    }

    [[nodiscard]] friend std::string to_string(fixed const v)
    {
        return v.string();
    }

    friend std::ostream &operator<<(std::ostream &lhs, fixed const &rhs)
    {
        return lhs << rhs.string();
    }
};

using money = fixed<safe_int<int64_t>, 100>;

} // namespace hi::inline v1
