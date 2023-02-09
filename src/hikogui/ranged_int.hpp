


#pragma once

#include "int_interval.hpp"
#include <limits>

namespace hi {
inline namspace v1 {
namespace detail {


}


template<interval<long long> Range>
struct ranged_int {
    using interval_type = interval<long long>;

    constexpr static interval_type range = Range;


    template<interval_type RHS>
    ranged_int(ranged_int<RHS> const &rhs) 
        noexcept(range.finite() and RHS.finite() and range.lo >= RHS.lo and RHS.hi <= range.hi) :
        _value(static_cast<value_type>(rhs._value))
    {
        if constexpr (not (range.finite() and RHS.finite() and range.lo >= RHS.lo and RHS.hi <= range.hi)) {
            if (rhs.value < range.lo or rhs.value > range.hi) {
                throw std::range_error("ranged_int");
            }
        }
    }

    template<interval_type RHS>
    [[nodiscard]] ranged_int<range + RHS> operator+(ranged_int<RHS> rhs) const noexcept(ranged_int<range + RHS>::finite())
    {
        using R = ranged_int<range + RHS>;

        if constexpr (R::finite()) {
            return R{_value + rhs._value, override_t{}};

        } else {
            auto lhs_ = wide_cast<long long>(_value); 
            auto rhs_ = wide_cast<long long>(rhs._value); 
            long long r = 0;
            if (overflow_add(lhs_, rhs_, r)) {
                throw std::overflow_error("ranged_int");
            }
            return R{r};
        }
    }

    template<interval_type RHS>
    [[nodiscard]] ranged_int<range - RHS> operator-(ranged_int<RHS> rhs) const noexcept(ranged_int<range - RHS>::finite())
    {
        using R = ranged_int<range - RHS>;

        if constexpr (R::finite()) {
            return R{_value - rhs._value, override_t{}};

        } else {
            auto lhs_ = wide_cast<long long>(_value); 
            auto rhs_ = wide_cast<long long>(rhs._value); 
            long long r = 0;
            if (overflow_sub(lhs_, rhs_, r)) {
                throw std::overflow_error("ranged_int");
            }
            return R{r};
        }
    }
};

template<fixed_string Literal>
auto operator "" _R()
{
    constexpr long long value = from_string_literal(Literal);

    return ranged_int<interval<value,value>>{value};
}



}}
