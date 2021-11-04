// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "string_tag.hpp"
#include "safe_int.hpp"
#include <type_traits>
#include <limits>

namespace tt::inline v1 {

template<typename T, int M>
struct fixed {
    using value_type = T;
    static constexpr int multiplier = M;

    T value;

    fixed() = default;
    ~fixed() = default;
    fixed(fixed const &) = default;
    fixed &operator=(fixed const &) = default;
    fixed(fixed &&) = default;
    fixed &operator=(fixed &&) = default;

    template<typename O, std::enable_if_t<std::is_floating_point_v<O>, int> = 0>  
    explicit constexpr fixed(O other) noexcept :
        value(static_cast<T>(other * M)) {
        tt_assert(
            other >= (std::numeric_limits<T>::min() / M) &&
            other <= (std::numeric_limits<T>::max() / M)
        );
    }

    template<typename O, std::enable_if_t<std::is_integral_v<O>, int> = 0>
    explicit constexpr fixed(O other) noexcept :
        value(static_cast<T>(other) * M) {
        tt_assert(
            other >= (std::numeric_limits<T>::min() / M) &&
            other <= (std::numeric_limits<T>::max() / M)
        );
    }

    explicit fixed(std::string const &other) :
        fixed(stod(other)) {}

    template<typename O, std::enable_if_t<std::is_floating_point_v<O>, int> = 0>  
    constexpr fixed &operator=(O other) noexcept {
        value = static_cast<T>(other * M);
        tt_assert(
            other >= (std::numeric_limits<T>::min() / M) &&
            other <= (std::numeric_limits<T>::max() / M)
        );
        return *this;
    }

    template<typename O, std::enable_if_t<std::is_integral_v<O>, int> = 0>
    constexpr fixed &operator=(O other) noexcept {
        value = static_cast<T>(other) * M;
        tt_assert(
            other >= (std::numeric_limits<T>::min() / M) &&
            other <= (std::numeric_limits<T>::max() / M)
        );
        return *this;
    }

    fixed &operator=(std::string const &other) {
        value = static_cast<T>(stod(other) * M);
        tt_assert(
            other >= (std::numeric_limits<T>::min() / M) &&
            other <= (std::numeric_limits<T>::max() / M)
        );
        return *this;
    }

    template<typename O, std::enable_if_t<std::is_floating_point_v<O>, int> = 0>  
    explicit operator O () const noexcept {
        return static_cast<O>(value) / M;
    }

    template<typename O, std::enable_if_t<std::is_integral_v<O>, int> = 0>
    explicit operator O () const noexcept {
        return static_cast<O>(value / M);
    }

    std::string string() const noexcept {
        return std::format("{}", static_cast<double>(value) / M);
    }
    
    static fixed fromValue(T value) noexcept {
        fixed r;
        r.value = value;
        return r;
    }
};

template<typename T, int M> inline bool operator==(fixed<T,M> const &lhs, fixed<T,M> const &rhs) { return lhs.value == rhs.value; }
template<typename T, int M> inline bool operator!=(fixed<T,M> const &lhs, fixed<T,M> const &rhs) { return lhs.value != rhs.value; }
template<typename T, int M> inline bool operator<(fixed<T,M> const &lhs, fixed<T,M> const &rhs) { return lhs.value < rhs.value; }
template<typename T, int M> inline bool operator>(fixed<T,M> const &lhs, fixed<T,M> const &rhs) { return lhs.value > rhs.value; }
template<typename T, int M> inline bool operator<=(fixed<T,M> const &lhs, fixed<T,M> const &rhs) { return lhs.value <= rhs.value; }
template<typename T, int M> inline bool operator>=(fixed<T,M> const &lhs, fixed<T,M> const &rhs) { return lhs.value >= rhs.value; }

template<typename T, int M> 
fixed<T,M> operator+(fixed<T,M> const &lhs, fixed<T,M> const &rhs)
{
    return fixed<T,M>::fromValue(lhs.value + rhs.value);
}

template<typename T, int M>
fixed<T,M> operator-(fixed<T,M> const &lhs, fixed<T,M> const &rhs)
{
    return fixed<T,M>::fromValue(lhs.value - rhs.value);
}

template<typename T, int M>
std::string to_string(fixed<T,M> const v)
{
    return v.string();
}

template<typename T, int M>
std::ostream &operator<<(std::ostream &lhs, fixed<T,M> const &rhs)
{
    return lhs << rhs.string();
}

using money = fixed<safe_int<int64_t>,100>;

}

