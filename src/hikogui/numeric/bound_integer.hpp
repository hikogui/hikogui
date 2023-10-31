// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "interval.hpp"
#include "../macros.hpp"
#include <type_traits>
#include <compare>
#include <stdexcept>

hi_export_module(hikogui.numeric.bound_integer);

hi_export namespace hi::inline v1 {

constexpr auto bounds_test = interval<long long>{1, 5};

static_assert(
    std::numeric_limits<signed long>::min() <= bounds_test.lower() and
    bounds_test.upper() <= std::numeric_limits<signed long>::max());

/** Bound integer.
 *
 */
template<interval<long long> Bounds>
struct bound_integer {
    using bound_type = interval<long long>;

    constexpr static bound_type bounds = Bounds;

    // clang-format off

    /** The type that can hold the value of the bound integer.
     */
    using value_type =
        std::conditional_t<bounds.type_contains_range<signed char>(), signed char,
        std::conditional_t<bounds.type_contains_range<unsigned char>(), unsigned char,
        std::conditional_t<bounds.type_contains_range<signed short>(), signed short,
        std::conditional_t<bounds.type_contains_range<unsigned short>(), unsigned short,
        std::conditional_t<bounds.type_contains_range<signed int>(), signed int,
        std::conditional_t<bounds.type_contains_range<unsigned int>(), unsigned int,
        std::conditional_t<bounds.type_contains_range<signed long>(), signed long,
        std::conditional_t<bounds.type_contains_range<unsigned long>(), unsigned long,
        signed long long>>>>>>>>;

    /** The type that is used as a temporary during calculation.
     */
    using calculation_type =
        std::conditional_t<bounds.type_contains_range<signed char>(), signed char,
        std::conditional_t<bounds.type_contains_range<signed short>(), signed short,
        std::conditional_t<bounds.type_contains_range<signed int>(), signed int,
        std::conditional_t<bounds.type_contains_range<signed long>(), signed long,
        signed long long>>>>;

    // clang-format on

    /** The value of the integer.
     */
    value_type value;

    constexpr bound_integer() noexcept : value(bounds.lower() <= 0 and 0 <= bounds.upper() ? 0 : bounds.lower()) {}
    constexpr bound_integer(bound_integer const &) noexcept = default;
    constexpr bound_integer(bound_integer &&) noexcept = default;
    constexpr bound_integer &operator=(bound_integer const &) noexcept = default;
    constexpr bound_integer &operator=(bound_integer &&) noexcept = default;

    [[nodiscard]] constexpr static bound_integer make_without_check(arithmetic auto other) noexcept
    {
        bound_integer r;
        r.value = static_cast<value_type>(other);
        hi_axiom(r.holds_invariant);
        return r;
    }

    constexpr bound_integer(arithmetic auto other) noexcept(bounds.range_contains_type<decltype(other)>()) :
        value(static_cast<value_type>(other))
    {
        if constexpr (not bounds.range_contains_type<decltype(other)>()) {
            if (other != bounds) {
                throw std::overflow_error("bound_integer(std::integral)");
            }
        }
        hi_axiom(holds_invariant());
    }

    constexpr bound_integer &operator=(arithmetic auto other) noexcept(bounds.range_contains_type<decltype(other)>())
    {
        if constexpr (not bounds.range_contains_type<decltype(other)>()) {
            if (other != bounds) {
                throw std::overflow_error("bound_integer(std::integral)");
            }
        }
        value = static_cast<value_type>(value);
        hi_axiom(holds_invariant());
        return *this;
    }

    template<bound_type OtherBound>
    constexpr bound_integer(bound_integer<OtherBound> other) noexcept(OtherBound.is_fully_inside(bounds)) :
        value(static_cast<value_type>(other.value))
    {
        if constexpr (not OtherBound.is_fully_inside(bounds)) {
            if (other.value != bounds) {
                throw std::overflow_error("bound_integer(bound_integer)");
            }
        }
        hi_axiom(holds_invariant());
    }

    template<bound_type OtherBound>
    constexpr bound_integer &operator=(bound_integer<OtherBound> other) noexcept(OtherBound.is_fully_inside(bounds))
    {
        if constexpr (not OtherBound.is_fully_inside(bounds)) {
            if (other.value != bounds) {
                throw std::overflow_error("bound_integer(bound_integer)");
            }
        }
        value = static_cast<value_type>(other._value);
        hi_axiom(holds_invariant());
        return *this;
    }

    //template<numeric_integral T>
    //explicit constexpr operator T() noexcept(values_between_bounds_fit_in_type_v<T>)
    //{
    //    if constexpr (not values_between_bounds_fit_in_type_v<T>) {
    //        if (not(std::numeric_limits<T>::min() <= value and value <= std::numeric_limits<T>::max())) {
    //            throw std::overflow_error("operator T()");
    //        }
    //    }
    //
    //    return static_cast<T>(value);
    //}

    explicit constexpr operator bool() noexcept
    {
        if constexpr (bounds.lower() > 0 or bounds.upper() < 0) {
            return true;
        } else if constexpr (bounds) {
            return value != value_type{0};
        } else {
            return false;
        }
    }

    [[nodiscard]] constexpr bool holds_invariant() noexcept
    {
        return value == bounds;
    }

    [[nodiscard]] auto operator-() const noexcept
    {
        using r_type = bound_integer<-bounds>;
        return r_type::make_without_check(-static_cast<r_type::calculation_type>(value));
    }

    /** Compare equality of two integers.
     */
    template<bound_type RHSBounds>
    [[nodiscard]] constexpr bool operator==(bound_integer<RHSBounds> const &rhs) noexcept
    {
        if constexpr (bounds.upper() < RHSBounds.lower() or bounds.lower() > RHSBounds.upper()) {
            return false;
        } else if (bounds.is_value() and RHSBounds.is_value() and bounds.lower() == RHSBounds.lower()) {
            return true;
        } else {
            return value == rhs.value;
        }
    }

    /** Compare two integers.
     */
    template<bound_type RHSBounds>
    [[nodiscard]] constexpr std::strong_ordering operator<=>(bound_integer<RHSBounds> const &rhs) noexcept
    {
        if constexpr (bounds.upper() < RHSBounds.lower()) {
            return std::strong_ordering::less;
        } else if constexpr (bounds.lower() > RHSBounds.upper()) {
            return std::strong_ordering::greater;
        } else if constexpr (bounds.is_value() and RHSBounds.is_value() and bounds.lower() == RHSBounds.lower()) {
            return std::strong_ordering::equal;
        } else {
            return value <=> rhs.value;
        }
    }

    template<bound_type RHSBounds>
    [[nodiscard]] constexpr auto operator+(bound_integer<RHSBounds> const &rhs) noexcept
    {
        static_assert(bounds.lower() >= (std::numeric_limits<long long>::min() >> 1), "lhs lower bound overflow");
        static_assert(bounds.upper() <= (std::numeric_limits<long long>::max() >> 1), "lhs upper bound overflow");
        static_assert(RHSBounds.lower() >= (std::numeric_limits<long long>::min() >> 1), "rhs lower bound overflow");
        static_assert(RHSBounds.upper() <= (std::numeric_limits<long long>::max() >> 1), "rhs upper bound overflow");

        using r_type = bound_integer<bounds + RHSBounds>;
        return r_type::make_without_check(
            static_cast<r_type::calculation_type>(value) + static_cast<r_type::calculation_type>(rhs.value));
    }

    template<bound_type RHSBounds>
    [[nodiscard]] constexpr auto operator-(bound_integer<RHSBounds> const &rhs) noexcept
    {
        static_assert(bounds.lower() >= (std::numeric_limits<long long>::min() >> 1), "lhs lower bound overflow");
        static_assert(bounds.upper() <= (std::numeric_limits<long long>::max() >> 1), "lhs upper bound overflow");
        static_assert(RHSBounds.lower() >= (std::numeric_limits<long long>::min() >> 1), "rhs lower bound overflow");
        static_assert(RHSBounds.upper() <= (std::numeric_limits<long long>::max() >> 1), "rhs upper bound overflow");

        using r_type = bound_integer<bounds - RHSBounds>;
        return r_type::make_without_check(
            static_cast<r_type::calculation_type>(value) - static_cast<r_type::calculation_type>(rhs.value));
    }

    template<bound_type RHSBounds>
    [[nodiscard]] constexpr auto operator*(bound_integer<RHSBounds> const &rhs) noexcept
    {
        static_assert(bounds.lower() >= (std::numeric_limits<intreg_t>::min()), "lhs lower bound overflow");
        static_assert(bounds.upper() <= (std::numeric_limits<intreg_t>::max()), "lhs upper bound overflow");
        static_assert(RHSBounds.lower() >= (std::numeric_limits<intreg_t>::min()), "rhs lower bound overflow");
        static_assert(RHSBounds.upper() <= (std::numeric_limits<intreg_t>::max()), "rhs upper bound overflow");

        using r_type = bound_integer<bounds * RHSBounds>;
        return r_type::make_without_check(
            static_cast<r_type::calculation_type>(value) * static_cast<r_type::calculation_type>(rhs.value));
    }

    template<bound_type RHSBounds>
    [[nodiscard]] constexpr auto operator/(bound_integer<RHSBounds> const &rhs) noexcept(0 != RHSBounds)
    {
        static_assert(bounds.lower() >= ((std::numeric_limits<long long>::min() + 1)), "lhs lower bound overflow");
        static_assert(bounds.upper() <= (std::numeric_limits<long long>::max()), "lhs upper bound overflow");
        static_assert(RHSBounds, "divide by zero");

        if constexpr (0 == RHSBounds) {
            if (rhs.value == 0) {
                throw std::domain_error("divide by zero");
            }
        }

        using r_type = bound_integer<bounds / RHSBounds>;
        return r_type::make_without_check(
            static_cast<r_type::calculation_type>(value) / static_cast<r_type::calculation_type>(rhs.value));
    }

    template<bound_type RHSBounds>
    [[nodiscard]] constexpr auto operator%(bound_integer<RHSBounds> const &rhs) noexcept(0 != RHSBounds)
    {
        static_assert(RHSBounds, "divide by zero");

        if constexpr (0 == RHSBounds) {
            if (rhs.value == 0) {
                throw std::domain_error("divide by zero");
            }
        }

        using r_type = bound_integer<bounds % RHSBounds>;
        return r_type::make_without_check(value % rhs.value);
    }

    /*
    template<long long lower_bound, long long upper_bound, long long RL, long long RU>
    [[nodiscard]] constexpr auto
    operator|(bound_integer<lower_bound, upper_bound> const &lhs, bound_integer<RL, RU> const &rhs) noexcept
    {
        constexpr auto a = lower_bound | RL;
        constexpr auto b = lower_bound | RU;
        constexpr auto c = upper_bound | RL;
        constexpr auto d = upper_bound | RU;
        constexpr auto r_lower_bound = std::min({a, b, c, d});
        constexpr auto r_upper_bound = std::max({a, b, c, d});
        using r_type = bound_integer<r_lower_bound, r_upper_bound>;
        using t_type = typename r_type::value_type;

        return rtype{static_cast<t_type>(lhs.value) | static_cast<t_type>(rhs.value)};
    }

    template<long long lower_bound, long long upper_bound, long long RL, long long RU>
    [[nodiscard]] constexpr auto
    operator&(bound_integer<lower_bound, upper_bound> const &lhs, bound_integer<RL, RU> const &rhs) noexcept
    {
        constexpr auto a = lower_bound & RL;
        constexpr auto b = lower_bound & RU;
        constexpr auto c = upper_bound & RL;
        constexpr auto d = upper_bound & RU;
        constexpr auto r_lower_bound = std::min({a, b, c, d});
        constexpr auto r_upper_bound = std::max({a, b, c, d});
        using r_type = bound_integer<r_lower_bound, r_upper_bound>;
        using t_type = typename r_type::value_type;

        return rtype{static_cast<t_type>(lhs.value) & static_cast<t_type>(rhs.value)};
    }

    template<long long lower_bound, long long upper_bound, long long RL, long long RU>
    [[nodiscard]] constexpr auto
    operator^(bound_integer<lower_bound, upper_bound> const &lhs, bound_integer<RL, RU> const &rhs) noexcept
    {
        // Use 'or' and 'and' for limits on 'xor'.
        constexpr auto a = lower_bound | RL;
        constexpr auto b = lower_bound | RU;
        constexpr auto c = upper_bound | RL;
        constexpr auto d = upper_bound | RU;
        constexpr auto e = lower_bound & RL;
        constexpr auto f = lower_bound & RU;
        constexpr auto g = upper_bound & RL;
        constexpr auto h = upper_bound & RU;
        constexpr auto r_lower_bound = std::min({a, b, c, d, e, f, g, h});
        constexpr auto r_upper_bound = std::max({a, b, c, d, e, f, g, h});
        using r_type = bound_integer<r_lower_bound, r_upper_bound>;
        using t_type = typename r_type::value_type;

        return rtype{static_cast<t_type>(lhs.value) ^ static_cast<t_type>(rhs.value)};
    }

    template<long long RL, long long RU>
    bound_integer &operator+=(bound_integer<RL, RU> const &rhs) noexcept(RL == 0 and RU == 0)
    {
        return *this = *this + rhs;
    }

    template<long long RL, long long RU>
    bound_integer &operator-=(bound_integer<RL, RU> const &rhs) noexcept(RL == 0 and RU == 0)
    {
        return *this = *this - rhs;
    }

    // clang-format off
    template<long long RL, long long RU>
    bound_integer &operator*=(bound_integer<RL, RU> const &rhs)
        noexcept(
            (RL == 1 and RU == 1) or
            (lower_bound <= 0 and upper_bound >= 0 and RL == 0 and RU == 0) or
            (lower_bound <= 0 and upper_bound >= 0 and RL == 0 and RU == 1) or
            (lower_bound == -upper_bound and RL == -1 and RU == -1) or
            (lower_bound <= 0 and upper_bound >= 0 and lower_bound == -upper_bound and RL == -1 and RU == 0) or
            (lower_bound <= 0 and upper_bound >= 0 and lower_bound == -upper_bound and RL == -1 and RU == 1) or
        )
    {
        return *this = *this * rhs;
    }
    // clang-format on

    template<long long RL, long long RU>
    bound_integer &operator/=(bound_integer<RL, RU> const &rhs) noexcept(RL > 0 or (lower_bound == -upper_bound and RU < 0))
    {
        return *this = *this / rhs;
    }

    template<long long RL, long long RU>
    bound_integer &operator%=(bound_integer<RL, RU> const &rhs) noexcept(RL > 0 or RU < 0)
    {
        return *this = *this % rhs;
    }

    template<long long RL, long long RU>
    bound_integer &operator&=(bound_integer<RL, RU> const &rhs) noexcept
    {
        value &= static_cast<value_type>(rhs.value);
        return *this;
    }

    template<long long RL, long long RU>
    bound_integer &operator|=(bound_integer<RL, RU> const &rhs)
    {
        return *this = *this | rhs;
    }

    template<long long RL, long long RU>
    bound_integer &operator^=(bound_integer<RL, RU> const &rhs)
    {
        return *this = *this ^ rhs;
    }

    [[nodiscard]] friend auto abs(bound_integer &rhs)
    {
        using std::abs;

        constexpr auto r_lower_bound = upper_bound < 0 ? -upper_bound : (lower_bound < 0 ? 0 : lower_bound);
        constexpr auto r_upper_bound = upper_bound >= 0 ? upper_bound : (lower_bound >= 0 ? lower_bound : 0);

        using r_type = bound_integer<lower_bound + RL, upper_bound + RU>;
        using t_type = typename r_type::value_type;

        if constexpr (lower_bound >= 0) {
            return r_type{static_cast<t_type>(rhs.value)};
        } else {
            if (rhs < 0) {
                return r_type{-static_cast<t_type>(rhs.value)};
            } else {
                return r_type{static_cast<t_type>(rhs.value)};
            }
        }
    }*/
};

// bound_integer(std::integral auto value)
//    -> bound_integer<std::numeric_limits<decltype(value)>::min(), std::numeric_limits<decltype(value)>::max()>;

// template<char... Chars>
// constexpr auto operator"" _I()
//{
//    constexpr long long value = long_long_from_chars<Chars...>();
//    return bound_integer_underlying<value, value>{value};
//}
} // namespace hi::inline v1
