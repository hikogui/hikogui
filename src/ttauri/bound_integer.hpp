
#pragma once

#include "concepts.hpp"
#include "register_int.hpp"

namespace tt {

/** Bound integer.
 *
 * @tparam L lower bound.
 * @tparam U upper bound.
 */
template<register_long L, register_long U>
struct bound_integer {
    static_assert(L <= H);

    constexpr long long lower_bound = L;
    constexpr long long upper_bound = U;

    /** Check if all the values of a type are within the bounds.
     */
    template<typename T>
    static constexpr bool values_of_type_are_within_bounds_v =
        lower_bound <= std::numeric_limits<T>::min() and std::numeric_limits<T>::max() <= upper_bound;

    /** Check if all the values between bounds can be represented by the type.
     */
    template<typename T>
    static constexpr bool values_between_bounds_fit_in_type_v = std::numeric_limits<T>::min() <= lower_bound and upper_bound
        <= std::numeric_limits<T>::max();

    // clang-format off
    using value_type =
        std::conditional_t<values_between_bounds_fit_in_type_v<signed char,L,U>, signed char,
        std::conditional_t<values_between_bounds_fit_in_type_v<signed short,L,U>, signed short,
        std::conditional_t<values_between_bounds_fit_in_type_v<signed int,L,U>, signed int,
        std::conditional_t<values_between_bounds_fit_in_type_v<signed long,L,U>, signed long,
        std::conditional_t<values_between_bounds_fit_in_type_v<signed long long,L,U>, signed long long,
        register_long>>>>>;
    // clang-format on

    /** The value of the integer.
     */
    value_type value;

    /** Check if the given value is within bounds.
     *
     * @param other The value to check.
     * @return True if @a other is between the `lower_bound` and `upper_bound`.
     */
    [[nodiscard]] static bool within_bounds(numeric_integral auto other) const noexcept
    {
        return lower_bound <= other and other <= upper_bound;
    }

    constexpr bound_integer() noexcept : value(0) {}
    constexpr bound_integer(bound_integer const &) noexcept = default;
    constexpr bound_integer(bound_integer &&) noexcept = default;
    constexpr bound_integer &operator=(bound_integer const &) noexcept = default;
    constexpr bound_integer &operator=(bound_integer &&) noexcept = default;

    constexpr bound_integer(numeric_integral auto other) noexcept(values_of_type_are_within_bounds_v<decltype(other)>) :
        value(static_cast<value_type>(other))
    {
        if constexpr (not values_of_type_are_within_bounds_v<decltype(other)>) {
            if (not within_bounds(other)) {
                throw std::overflow_error("bound_integer(std::integral)")
            }
        }
        tt_axiom(holds_invariant());
    }

    constexpr bound_integer &operator=(numeric_integral auto other) noexcept(values_of_type_are_within_bounds_v<decltype(other)>)
    {
        if constexpr (not values_of_type_are_within_bounds_v<decltype(other)>) {
            if (not within_bounds(other)) {
                throw std::overflow_error("bound_integer(std::integral)")
            }
        }
        value = static_cast<value_type>(value);
        tt_axiom(holds_invariant());
        return *this;
    }

    template<register_long OL, register_long OU>
    constexpr bound_integer(bound_integer<OL, OU> other) noexcept(lower_bound <= OL and OH <= upper_bound) :
        _value(static_cast<value_type>(other.value))
    {
        if constexpr (not(lower_bound <= OL and OH <= upper_bound)) {
            if (not within_bounds(other.value)) {
                throw std::overflow_error("bound_integer(bound_integer)")
            }
        }
        tt_axiom(holds_invariant());
    }

    template<register_long OL, register_long OH>
    constexpr bound_integer &operator=(bound_integer<OL, OH> other) noexcept(lower_bound <= OL and OH <= upper_bound)
    {
        if constexpr (not(lower_bound <= OL and OH <= upper_bound)) {
            if (not within_bounds(other.value)) {
                throw std::overflow_error("bound_integer(bound_integer)")
            }
        }
        value = static_cast<value_type>(other._value);
        tt_axiom(holds_invariant());
        return *this;
    }

    template<numeric_integral T>
    explicit constexpr operator T() noexcept(values_between_bounds_fit_in_type_v<T>)
    {
        if constexpr (not values_between_bounds_fit_in_type_v<T>) {
            if (not(std::numeric_limits<T>::min() <= value and value <= std::numeric_limits<T>::max())) {
                throw std::overflow_error("operator T()");
            }
        }

        return static_cast<T>(_value);
    }

    explicit constexpr operator bool() noexcept
    {
        if constexpr (lower_bound > 0 or upper_bound < 0) {
            return true;
        } else if constexpr (lower_bound == 0 and upper_bound == 0) {
            return false;
        } else {
            return value != value_type{0};
        }
    }

    [[nodiscard]] constexpr bool holds_invariant() noexcept
    {
        return lower_bound <= value and value <= upper_bound;
    }

    [[nodiscard]] auto operator-() const noexcept
    {
        using r_type = bound_integer<-upper_bound, -lower_bound>;
        using t_type = typename r_type::value_type;
        return r_type{-static_cast<t_type>(value)};
    }

    /** Compare equality of two integers.
     */
    template<register_long RL, register_long RU>
    [[nodiscard]] constexpr bool operator==(bound_integer<RL, RU> const &rhs) noexcept
    {
        if constexpr (upper_bound < RL or lower_bound > RU) {
            return false;
        } else if (lower_bound == upper_bound and lower_bound == RL and upper_bound == RU) {
            return true;
        } else {
            return value == rhs.value;
        }
    }

    /** Compare two integers.
     */
    template<register_long RL, register_long RU>
    [[nodiscard]] constexpr std::strong_ordering operator<=>(bound_integer<RL, RU> const &rhs) noexcept
    {
        if constexpr (upper_bound < RL) {
            return std::strong_ordering::less;
        } else if constexpr (lower_bound > RU) {
            return std::strong_ordering::greater;
        } else if constexpr (lower_bound == upper_bound and lower_bound == RL and upper_bound == RU) {
            return std::strong_ordering::equal;
        } else {
            return value <=> rhs.value;
        }
    }

    template<register_long RL, register_long RU>
    [[nodiscard]] constexpr auto operator+(bound_integer<RL, RU> const &rhs) noexcept
    {
        static_assert(lower_bound >= (std::numeric_limits<register_long>::min() >> 1), "lhs lower bound overflow");
        static_assert(upper_bound <= (std::numeric_limits<register_long>::max() >> 1), "lhs upper bound overflow");
        static_assert(RL >= (std::numeric_limits<register_long>::min() >> 1), "rhs lower bound overflow");
        static_assert(RU <= (std::numeric_limits<register_long>::max() >> 1), "rhs upper bound overflow");

        using r_type = bound_integer<lower_bound + RL, upper_bound + RU>;
        using t_type = typename r_type::value_type;

        return rtype{static_cast<t_type>(lhs.value) + static_cast<t_type>(rhs.value)};
    }

    template<register_long RL, register_long RU>
    [[nodiscard]] constexpr auto operator-(bound_integer<RL, RU> const &rhs) noexcept
    {
        static_assert(lower_bound >= (std::numeric_limits<register_long>::min() >> 1), "lhs lower bound overflow");
        static_assert(upper_bound <= (std::numeric_limits<register_long>::max() >> 1), "lhs upper bound overflow");
        static_assert(RL >= (std::numeric_limits<register_long>::min() >> 1), "rhs lower bound overflow");
        static_assert(RU <= (std::numeric_limits<register_long>::max() >> 1), "rhs upper bound overflow");

        using r_type = bound_integer<lower_bound - RU, upper_bound - RL>;
        using t_type = typename r_type::value_type;

        return rtype{static_cast<t_type>(lhs.value) - static_cast<t_type>(rhs.value)};
    }

    template<register_long RL, register_long RU>
    [[nodiscard]] constexpr auto operator*(bound_integer<RL, RU> const &rhs) noexcept
    {
        static_assert(lower_bound >= (std::numeric_limits<register_int>::min()), "lhs lower bound overflow");
        static_assert(upper_bound <= (std::numeric_limits<register_int>::max()), "lhs upper bound overflow");
        static_assert(RL >= (std::numeric_limits<register_int>::min()), "rhs lower bound overflow");
        static_assert(RU <= (std::numeric_limits<register_int>::max()), "rhs upper bound overflow");

        constexpr auto a = lower_bound * RL;
        constexpr auto b = lower_bound * RU;
        constexpr auto c = upper_bound * RL;
        constexpr auto d = upper_bound * RU;
        constexpr auto r_lower_bound = std::min({a, b, c, d});
        constexpr auto r_upper_bound = std::max({a, b, c, d});

        using r_type = bound_integer<r_lower_bound, r_upper_bound>;
        using t_type = typename r_type::value_type;

        return rtype{static_cast<t_type>(lhs.value) * static_cast<t_type>(rhs.value)};
    }

    template<register_long RL, register_long RU>
    [[nodiscard]] constexpr auto operator/(bound_integer<RL, RU> const &rhs) noexcept(RL > 0 or RU < 0)
    {
        static_assert(lower_bound >= ((std::numeric_limits<register_long>::min() + 1)), "lhs lower bound overflow");
        static_assert(upper_bound <= (std::numeric_limits<register_long>::max()), "lhs upper bound overflow");

        // Since divide by zero is not allowed, the limits are as if divided by -1 or 1.
        constexpr auto a = RL ? (lower_bound / RL) : (RL < 0 : -lower_bound : lower_bound);
        constexpr auto b = RU ? (lower_bound / RU) : (RU < 0 : -lower_bound : lower_bound);
        constexpr auto c = RL ? (upper_bound / RL) : (RL < 0 : -upper_bound : upper_bound);
        constexpr auto d = RU ? (upper_bound / RU) : (RU < 0 : -upper_bound : upper_bound);
        constexpr auto r_lower_bound = std::min({a, b, c, d});
        constexpr auto r_upper_bound = std::max({a, b, c, d});

        using r_type = bound_integer<r_lower_bound, r_upper_bound>;
        using t_type = typename r_type::value_type;

        static_assert(RL == 0 and RU == 0, "divide by zero");
        if constexpr (not(RL > 0 or RU < 0)) {
            if (rhs.value == 0) {
                throw std::domain_error("divide by zero");
            }
        }

        return rtype{static_cast<t_type>(lhs.value) / static_cast<t_type>(rhs.value)};
    }

    template<register_long lower_bound, register_long upper_bound, register_long RL, register_long RU>
    [[nodiscard]] constexpr auto
    operator%(bound_integer<lower_bound, upper_bound> const &lhs, bound_integer<RL, RU> const &rhs) noexcept(RL > 0 or RU < 0)
    {
        constexpr auto t_lower_bound = std::min(lower_bound < 0 ? -RU : RL, lower_bound);
        constexpr auto t_upper_bound = std::max(upper_bound < 0 ? -RL : RU, upper_bound);
        using t_type = typename bound_integer<t_lower_bound, t_upper_bound>::value_type;

        constexpr auto r_lower_bound = std::max(lower_bound < 0 ? -RU : RL, lower_bound);
        constexpr auto r_upper_bound = std::min(upper_bound < 0 ? -RL : RU, upper_bound);
        using r_type = bound_integer<r_lower_bound, r_upper_bound>;

        static_assert(RL == 0 and RU == 0, "divide by zero");
        if (not(RL > 0 or RU < 0)) {
            if (rhs.value == 0) {
                throw std::domain_error("divide by zero");
            }
        }

        return rtype{static_cast<t_type>(lhs.value) % static_cast<t_type>(rhs.value)};
    }

    template<register_long lower_bound, register_long upper_bound, register_long RL, register_long RU>
    [[nodiscard]] constexpr auto operator|(bound_integer<lower_bound, upper_bound> const &lhs, bound_integer<RL, RU> const &rhs) noexcept
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

    template<register_long lower_bound, register_long upper_bound, register_long RL, register_long RU>
    [[nodiscard]] constexpr auto operator&(bound_integer<lower_bound, upper_bound> const &lhs, bound_integer<RL, RU> const &rhs) noexcept
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

    template<register_long lower_bound, register_long upper_bound, register_long RL, register_long RU>
    [[nodiscard]] constexpr auto operator^(bound_integer<lower_bound, upper_bound> const &lhs, bound_integer<RL, RU> const &rhs) noexcept
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

    template<register_long RL, register_long RU>
    bound_integer &operator+=(bound_integer<RL, RU> const &rhs) noexcept (RL == 0 and RU == 0)
    {
        return *this = *this + rhs;
    }

    template<register_long RL, register_long RU>
    bound_integer &operator-=(bound_integer<RL, RU> const &rhs) noexcept(RL == 0 and RU == 0)
    {
        return *this = *this - rhs;
    }

    // clang-format off
    template<register_long RL, register_long RU>
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

    template<register_long RL, register_long RU>
    bound_integer &operator/=(bound_integer<RL, RU> const &rhs) noexcept(RL > 0 or (lower_bound == -upper_bound and RU < 0))
    {
        return *this = *this / rhs;
    }

    template<register_long RL, register_long RU>
    bound_integer &operator%=(bound_integer<RL, RU> const &rhs) noexcept(RL > 0 or RU < 0)
    {
        return *this = *this % rhs;
    }

    template<register_long RL, register_long RU>
    bound_integer &operator&=(bound_integer<RL, RU> const &rhs) noexcept
    {
        value &= static_cast<value_type>(rhs.value);
        return *this;
    }

    template<register_long RL, register_long RU>
    bound_integer &operator|=(bound_integer<RL, RU> const &rhs)
    {
        return *this = *this | rhs;
    }

    template<register_long RL, register_long RU>
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
    }
};

bound_integer(std::integral auto value)
    -> bound_integer<std::numeric_limits<decltype(value)>::min(), std::numeric_limits<decltype(value)>::max()>;

template<char... Chars>
constexpr auto operator"" _I()
{
    constexpr long long value = long_long_from_chars<Chars...>();
    return bound_integer_underlying<value, value>{value};
}
} // namespace tt
