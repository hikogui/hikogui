


#pragma once

#include "int_interval.hpp"

namespace hi {
inline namspace v1 {


template<interval<long long> Range>
struct ranged_int {


    template<long long 
    auto operator+(ranged_int<>
};

template<fixed_string Literal>
auto operator "" _R()
{
    constexpr long long value = from_string_literal(Literal);

    return ranged_int<interval<value,value>>{value};
}



}}
