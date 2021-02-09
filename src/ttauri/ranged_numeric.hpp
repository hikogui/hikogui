// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <limits>
#include <ranges>
#include "cast.hpp"

namespace tt {

/** A ranged integer.
 * This is an integer value that must stay within its minimum and maximum value.
 * 
 * @tparam A When only one parameter is given one beyond the maximum value.
 *           When both parameters are given the minimum value.
 * @tparam B One beyond the maximum value.
 */
template<typename T, T A, T B=A>
class ranged_numeric {
public:
    using value_type = T;
    static constexpr int value_min = (B == A) ? 0 : A;
    static constexpr int value_max = (B == A) ? A : B;

    static_assert(value_min < value_max);

    constexpr ranged_numeric() noexcept : value(value_min) {}
    constexpr ranged_numeric(ranged_numeric const &rhs) noexcept = default;
    constexpr ranged_numeric(ranged_numeric &&rhs) noexcept = default;
    constexpr ranged_numeric &operator=(ranged_numeric const &rhs) noexcept = default;
    constexpr ranged_numeric &operator=(ranged_numeric &&rhs) noexcept = default;

    constexpr explicit operator value_type () noexcept
    {
        return value;
    }

    [[nodiscard]] constexpr ranged_numeric(signed long long rhs) noexcept : value(narrow_cast<T>(rhs))
    {
        tt_axiom(value >= value_min && value < value_max);
    }

    [[nodiscard]] constexpr ranged_numeric(unsigned long long rhs) noexcept : value(narrow_cast<T>(rhs))
    {
        tt_axiom(value >= value_min && value < value_max);
    }

    [[nodiscard]] constexpr ranged_numeric(signed long rhs) noexcept : value(narrow_cast<T>(rhs))
    {
        tt_axiom(value >= value_min && value < value_max);
    }

    [[nodiscard]] constexpr ranged_numeric(unsigned long rhs) noexcept : value(narrow_cast<T>(rhs))
    {
        tt_axiom(value >= value_min && value < value_max);
    }

    [[nodiscard]] constexpr ranged_numeric(signed int rhs) noexcept : value(narrow_cast<T>(rhs))
    {
        tt_axiom(value >= value_min && value < value_max);
    }

    [[nodiscard]] constexpr ranged_numeric(unsigned int rhs) noexcept : value(narrow_cast<T>(rhs))
    {
        tt_axiom(value >= value_min && value < value_max);
    }

    [[nodiscard]] constexpr ranged_numeric(signed short rhs) noexcept : value(narrow_cast<T>(rhs))
    {
        tt_axiom(value >= value_min && value < value_max);
    }

    [[nodiscard]] constexpr ranged_numeric(unsigned short rhs) noexcept : value(narrow_cast<T>(rhs))
    {
        tt_axiom(value >= value_min && value < value_max);
    }

    [[nodiscard]] constexpr ranged_numeric(signed char rhs) noexcept : value(narrow_cast<T>(rhs))
    {
        tt_axiom(value >= value_min && value < value_max);
    }

    [[nodiscard]] constexpr ranged_numeric(unsigned char rhs) noexcept : value(narrow_cast<T>(rhs))
    {
        tt_axiom(value >= value_min && value < value_max);
    }

    [[nodiscard]] constexpr ranged_numeric &operator++() noexcept
    {
        tt_axiom(value < value_max - 1);
        ++value;
        return *this;
    }

    [[nodiscard]] constexpr ranged_numeric &operator--() noexcept
    {
        tt_axiom(value > value_min);
        --value;
        return *this;
    }

    [[nodiscard]] friend bool operator==(ranged_numeric const &lhs, ranged_numeric const &rhs) noexcept
    {
        return lhs.value == rhs.value;
    }

    [[nodiscard]] friend bool operator!=(ranged_numeric const &lhs, ranged_numeric const &rhs) noexcept
    {
        return lhs.value != rhs.value;
    }

    [[nodiscard]] friend bool operator<(ranged_numeric const &lhs, ranged_numeric const &rhs) noexcept
    {
        return lhs.value < rhs.value;
    }

    [[nodiscard]] friend bool operator>(ranged_numeric const &lhs, ranged_numeric const &rhs) noexcept
    {
        return lhs.value > rhs.value;
    }

    [[nodiscard]] friend bool operator<=(ranged_numeric const &lhs, ranged_numeric const &rhs) noexcept
    {
        return lhs.value <= rhs.value;
    }

    [[nodiscard]] friend bool operator>=(ranged_numeric const &lhs, ranged_numeric const &rhs) noexcept
    {
        return lhs.value >= rhs.value;
    }

    [[nodiscard]] static constexpr auto range() noexcept
    {
        tt_assert(value_max - value_min < 10, "Don't use this until std::views::iota() is implemented");

        std::vector<ranged_numeric> r;
        r.reserve(value_max - value_min);
        for (auto i = value_min; i != value_max; ++i) {
            r.push_back(i);
        }

        return r;

        //return 
        //    std::views::iota(value_min, value_max) | std::views::transform([](auto &&x) {
        //           return ranged_numeric{x};
        //       });
    }

private:
    int value;
};

template<int A, int B = A>
using ranged_int = ranged_numeric<int,A,B>;

}

namespace std {

template<typename T, int A, int B>
class numeric_limits<tt::ranged_numeric<T,A,B>> {
    static constexpr auto is_specialized = true;
    static constexpr auto is_signed = std::numeric_limits<T>::is_signed;
    static constexpr auto is_integer = std::numeric_limits<T>::is_integer;
    static constexpr auto is_exact = std::numeric_limits<T>::is_exact;
    static constexpr auto has_infinity = std::numeric_limits<T>::has_infinity;
    static constexpr auto has_quiet_NaN = std::numeric_limits<T>::has_quiet_NaN;
    static constexpr auto has_signaling_NaN = std::numeric_limits<T>::has_signaling_NaN;
    static constexpr auto has_denorm = std::numeric_limits<T>::has_denorm;
    static constexpr auto has_denorm_loss = std::numeric_limits<T>::has_denorm_loss;
    static constexpr auto round_style = std::numeric_limits<T>::round_style;
    static constexpr auto is_iec559 = std::numeric_limits<T>::is_iec559;
    static constexpr auto is_bounded = std::numeric_limits<T>::is_bounded;
    static constexpr auto is_modulo = std::numeric_limits<T>::is_modulo;
    static constexpr auto digits = std::numeric_limits<T>::digits;
    static constexpr auto digits10 = std::numeric_limits<T>::digits10;
    static constexpr auto max_digits10 = std::numeric_limits<T>::max_digits10;
    static constexpr auto radix = std::numeric_limits<T>::radix;
    static constexpr auto min_exponent = std::numeric_limits<T>::min_exponent;
    static constexpr auto min_exponent10 = std::numeric_limits<T>::min_exponent10;
    static constexpr auto max_exponent = std::numeric_limits<T>::max_exponent;
    static constexpr auto max_exponent10 = std::numeric_limits<T>::max_exponent10;
    static constexpr auto traps = std::numeric_limits<T>::traps;
    static constexpr auto tinyness_before = std::numeric_limits<T>::tinyness_before;

    static constexpr auto min() noexcept
    {
        if constexpr (is_floating_point_v<T> && tt::ranged_int<T, A, B>::value_min < std::numeric_limits<T>::min()) {
            return std::numeric_limits<T>::min();
        } else {
            return tt::ranged_int<T, A, B>::value_min;
        }
    }

    static constexpr auto lowest() noexcept
    {
        return tt::ranged_int<T, A, B>::value_min;
    }

    static constexpr auto max() noexcept
    {
        return tt::ranged_int<T, A, B>::value_max;
    }

    static constexpr auto epsilon() noexcept
    {
        return std::numeric_limits<T>::epsilon();
    }

    static constexpr auto round_error() noexcept
    {
        return std::numeric_limits<T>::round_error();
    }

    static constexpr auto infinity() noexcept
    {
        return std::numeric_limits<T>::infinity();
    }

    static constexpr auto quiet_NaN() noexcept
    {
        return std::numeric_limits<T>::quiet_NaN();
    }

    static constexpr auto signaling_NaN() noexcept
    {
        return std::numeric_limits<T>::signaling_NaN();
    }

    static constexpr auto denorm_min() noexcept
    {
        if constexpr (is_floating_point_v<T> && tt::ranged_int<T, A, B>::value_min < std::numeric_limits<T>::denorm_min()) {
            return std::numeric_limits<T>::denorm_min();
        } else {
            return tt::ranged_int<T, A, B>::value_min;
        }
    }
};

}
