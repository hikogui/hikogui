
#pragma once

#include "register_int.hpp"

namespace tt {

template<register_long L, register_long U>
class bound_integer {
public:
    static_assert(L <= H);

    constexpr long long lower = L;
    constexpr long long upper = U;

    /** Check if all the values of a type are within the bounds.
     */
    template<typename T>
    static constexpr bool values_of_type_are_within_bounds_v =
        lower <= std::numeric_limits<T>::min() and std::numeric_limits<T::max() <= upper;

    /** Check if all the values between bounds can be represented by the type.
     */
    template<typename T>
    static constexpr bool values_between_bounds_fit_in_type_v =
        std::numeric_limits<T>::min() <= lowest and std::numeric_limits<T::max() >= highest;


    using value_type =
        std::conditional_t<values_between_bounds_fit_in_type_v<signed char,L,U>, signed char,
        std::conditional_t<values_between_bounds_fit_in_type_v<short,L,U>, short,
        std::conditional_t<values_between_bounds_fit_in_type_v<int,L,U>, int,
        std::conditional_t<values_between_bounds_fit_in_type_v<long,L,U>, long,
        std::conditional_t<values_between_bounds_fit_in_type_v<long long,L,U>, long long,
        register_long>>>>>;


    constexpr bound_integer() noexcept : _value(std::numeric_limits<value_type>::min()) {}
    constexpr bound_integer(bound_integer const &) noexcept = default;
    constexpr bound_integer(bound_integer &&) noexcept = default;
    constexpr bound_integer &operator=(bound_integer const &) noexcept = default;
    constexpr bound_integer &operator=(bound_integer &&) noexcept = default;

    template<std::integral T>
    constexpr bound_integer(T value) noexcept(integral_inside_domain_v<T>):
        _value(static_cast<value_type>(value))
    {
        if constexpr (not integral_inside_domain_v<T>) {
            if (value < lowest or value > highest) {
                throw std::domain_error("bound_integer(std::integral)")
            }
        }
        tt_axiom(holds_invariant());
    }

    template<std::integral T>
    constexpr bound_integer &operator=(T value) noexcept(integral_inside_domain_v<T>)
    {
        if constexpr (not integral_inside_domain_v<T>) {
            if (value < lowest or value > highest) {
                throw std::domain_error("bound_integer(std::integral)")
            }
        }
        _value = static_cast<value_type>(value)
        tt_axiom(holds_invariant());
        return *this;
    }

    template<long long OL, long long OH>
    constexpr bound_integer(bound_integer<OL,OH> other) noexcept (OL >= lowest and OH <= highest) :
        _value(static_cast<value_type>(other._value))
    {
        if constexpr (OL < lowest or OH > highest) {
            if (other._value < lowest or other._value > highest) {
                throw std::domain_error("bound_integer(bound_integer)")
            }
        }
        tt_axiom(holds_invariant());
    }

    template<long long OL, long long OH>
    constexpr bound_integer &operator=(bound_integer<OL,OH> other) noexcept (OL >= lowest and OH <= highest)
    {
        if constexpr (OL < lowest or OH > highest) {
            if (other._value < lowest or other._value > highest) {
                throw std::domain_error("bound_integer(bound_integer)")
            }
        }
        value = static_cast<value_type>(other._value);
        tt_axiom(holds_invariant());
        return *this;
    }

    template<std::integral T>
    explicit constexpr operator T () noexcept(integral_holds_domain_v<T>)
    {
        if constexpr (not integral_holds_domain_v<T>) {
            if (_value < std::numeric_limits<T>::min() or _value > std::numeric_limits<T>::max())
                throw std::domain_error("operator T()");
            }
        }
        return static_cast<T>(_value);
    }

    explicit constexpr operator bool () noexcept
    {
        if constexpr (lowest > 0 or highest < 0) {
            return true;
        } else if constexpr (lowest == 0 and highest == 0) {
            return false;
        } else {
            return _value != 0;
        }
    }

    [[nodiscard]] constexpr bool holds_invariant() noexcept
    {
        return _value >= L and _value <= R;
    }

    /** Compare equality of an bound_integer with another bound_integer.
     */
    template<long long OL, long long OH>
    [[nodiscard]] constexpr bool operator==(bound_integer<OL,OH> const &rhs) noexcept
    {
        if constexpr (OL > highest or OH < lowest) {
            return false;
        } else if (OL == OH and lowest == highest and OL == lowest) {
            return true;
        } else {
            return _value == rhs._value;
        }
    }

    /** Compare an bound_integer with another bound_integer.
     */
    template<long long OL, long long OH>
    [[nodiscard]] constexpr std::strong_ordering operator<=>(bound_integer<OL,OH> const &rhs) noexcept
    {
        if constexpr (highest < OL ) {
            return std::strong_ordering::less;
        } else if constexpr (lowest > OH) {
            return std::strong_ordering::greater;
        } else if constexpr (OL == OH and lowest == highest and OL == lowest) {
            return std::strong_ordering::equal;
        } else{
            return _value <=> rhs._value;
        }
    }

    template<long long OL, long long OH>
    [[nodiscard]] constexpr auto operator+(bound_integer<OL,OH> const &rhs) noexcept
    {
        return bound_integer<lowest+OL,highest+OH>{_value + rhs};
    }

    template<long long OL, long long OH>
    [[nodiscard]] constexpr auto operator-(bound_integer<OL,OH> const &rhs) noexcept
    {
        return bound_integer<lowest-OL,highest-OH>{_value - rhs};
    }


private:
    value_type _value; 

    template<long long FL, long long FH>
    friend class bound_integer<FL, FH>;
};

bound_integer(std::integral auto value) ->
    bound_integer<std::numeric_limits<decltype(value)>::min(),std::numeric_limits<decltype(value)>::max()>;

template<char... Chars>
constexpr auto operator "" _I()
{
    constexpr long long value = long_long_from_chars<Chars...>();
    return bound_integer_underlying<value,value>{value};
}


}

