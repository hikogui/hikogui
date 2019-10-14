// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/string_tag.hpp"
#include <type_traits>
#include <limits>

namespace TTauri {

template<typename T, int M, string_tag Tag=0>
struct fixed {
    using value_type = T;
    constexpr int multiplier = M;
    constexpr string_tag tag = Tag;

    T value;

    fixed() = default;
    ~fixed() = default;
    fixed(fixed const &) = default;
    fixed &operator=(fixed const &) = default;
    fixed(fixed &&) = default;
    fixed &operator=(fixed &&) = default;

    template<typename O, std::enabled_if_t<std::is_float_v<O>, int> = 0>  
    explicit constexpr fixed(O &other) noexcept :
        value(static_cast<T>(other * M)) {
        required_assert(
            other >= (std::numeric_limits<T>::min() / M)
            other <= (std::numeric_limits<T>::max() / M)
        );
    }

    template<typename O, std::enabled_if_t<std::is_integral_v<O>, int> = 0>
    explicit constexpr fixed(O const &other) noexcept :
        value(static_cast<T>(other) * M) {
        required_assert(
            other >= (std::numeric_limits<T>::min() / M)
            other <= (std::numeric_limits<T>::max() / M)
        );
    }

    template<typename O, std::enabled_if_t<std::is_float_v<O>, int> = 0>  
    explicit constexpr fixed &operator=(long double const &other) noexcept {
        value = static_cast<T>(other * M);
        return *this;
        required_assert(
            other >= (std::numeric_limits<T>::min() / M)
            other <= (std::numeric_limits<T>::max() / M)
        );
    }

    template<typename O, std::enabled_if_t<std::is_integral_v<O>, int> = 0>
    explicit constexpr fixed &operator=(O const &other) noexcept {
        value = static_cast<T>(other) * M;
        return *this;
        required_assert(
            other >= (std::numeric_limits<T>::min() / M)
            other <= (std::numeric_limits<T>::max() / M)
        );
    }

    template<typename O, std::enabled_if_t<std::is_float_v<O>, int> = 0>  
    explicit operator O () const noexcept {
        return static_cast<O>(value) / M;
    }

    template<typename O, std::enabled_if_t<std::is_integral_v<O>, int> = 0>
    explicit operator O () const noexcept {
        return static_cast<O>(value / M);
    }
    
    static fixed fromValue(T value) noexcept {
        fixed r;
        r.value = value;
        return r;
    }
};


inline bool operator==(fixed const &lhs, fixed const &rhs) { return lhs.value == rhs.value; }
inline bool operator!=(fixed const &lhs, fixed const &rhs) { return lhs.value != rhs.value; }
inline bool operator<(fixed const &lhs, fixed const &rhs) { return lhs.value < rhs.value; }
inline bool operator>(fixed const &lhs, fixed const &rhs) { return lhs.value > rhs.value; }
inline bool operator<=(fixed const &lhs, fixed const &rhs) { return lhs.value <= rhs.value; }
inline bool operator>=(fixed const &lhs, fixed const &rhs) { return lhs.value >= rhs.value; }

fixed operator+(fixed const &lhs, fixed const &rhs)
{
    return fixed::fromValue(lhs.value + rhs.value);
}

fixed operator-(fixed const &lhs, fixed const &rhs)
{
    return fixed::fromValue(lhs.value - rhs.value);
}

fixed operator*(fixed const &lhs, fixed const &rhs)
{
    return fixed::fromValue((lhs.value * rhs.value) / 
}

#define BIOPCAST(x)\
    template<typename O> inline fixed operator x (fixed const &lhs, O const &rhs) { return lhs x fixed{rhs}; }\
    template<typename O> inline fixed operator x (O const &lhs, fixed const &rhs) { return fixed{lhs} x rhs; }

BIOPCAST(+)
BIOPCAST(-)

#define BICMPCAST(x)\
    template<typename O> inline bool operator x (fixed const &lhs, O const &rhs) { return lhs x fixed{rhs}; }\
    template<typename O> inline bool operator x (O const &lhs, fixed const &rhs) { return fixed{lhs} x rhs; }

BICMPCAST(==)
BICMPCAST(!=)
BICMPCAST(<)
BICMPCAST(>)
BICMPCAST(<=)
BICMPCAST(>=)

#undef BICMPCAST
#undef BIOPCAST
}

