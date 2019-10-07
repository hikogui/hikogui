// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/int_overflow.hpp"
#include "TTauri/Required/type_traits.hpp"
#include <type_traits>
#include <limits>

namespace TTauri {

enum class on_overflow_t {
    Throw,
    Terminate,
    Saturate
};

template<typename T, on_overflow_t OnOverflow>
inline T safe_overflow(char const *message, bool is_max)
{
    if constexpr (OnOverflow == on_overflow_t::Throw) {
        TTAURI_THROW(overflow_error(message));
    } else if constexpr (OnOverflow == on_overflow_t::Terminate) {
        axiom_assert(true);
    } else if constexpr (OnOverflow == on_overflow_t::Saturate) {
        return is_max ? std::numeric_limits<T>::max() : std::numeric_limits<T>::min();
    }
}

template<typename T, on_overflow_t OnOverflow, typename U>
inline T safe_convert(U const &rhs)
{
    T r;
    // Optimized away when is_same_v<T,U>
    if (convert_overflow(rhs, &r)) {
        return safe_overflow<T,OnOverflow>("safe_convert", rhs >= 0);
    }
    return r;
}

template<on_overflow_t OnOverflow, typename T, typename U>
inline make_promote_t<T,U> safe_add(T const &lhs, U const &rhs)
{
    make_promote_t<T,U> r;
    let lhs_ = static_cast<make_promote_t<T,U>>(lhs);
    let rhs_ = static_cast<make_promote_t<T,U>>(rhs);

    T r;
    if (add_overflow(lhs, rhs, &r)) {
        return safe_overflow<make_promote_t<T,U>,OnOverflow>("safe_add", lhs >= 0);
    }
    return r;
}

template<on_overflow_t OnOverflow, typename T, typename U>
inline make_promote_t<T,U> safe_sub(T const &lhs, U const &rhs)
{
    make_promote_t<T,U> r;
    let lhs_ = static_cast<make_promote_t<T,U>>(lhs);
    let rhs_ = static_cast<make_promote_t<T,U>>(rhs);

    T r;
    if (sub_overflow(lhs, rhs, &r)) {
        return safe_overflow<make_promote_t<T,U>,OnOverflow>("safe_sub", lhs >= 0);
    }
    return r;
}

template<on_overflow_t OnOverflow, typename T, typename U>
inline make_promote_t<T,U> safe_mul(T const &lhs, U const &rhs)
{
    make_promote_t<T,U> r;
    let lhs_ = static_cast<make_promote_t<T,U>>(lhs);
    let rhs_ = static_cast<make_promote_t<T,U>>(rhs);

    T r;
    if (mul_overflow(lhs, rhs, &r)) {
        return safe_overflow<make_promote_t<T,U>,OnOverflow>("safe_mul", (lhs_ ^ rhs_) >= 0);
    }
    return r;
}


template<typename T, on_overflow_t OnOverflow=on_overflow_t::Terminate>
struct safe_int {
    using value_type = T;
    constexpr on_overflow_t on_overflow = OnOverflow;

    T value;

    safe_int() : value(0) {}
    ~safe_int() = default;
    safe_int(safe_int const &) = default;
    safe_int(safe_int &&) = default;
    safe_int &operator(safe_int const &) = default;
    safe_int &operator(safe_int &&) = default;

    template<typename O, std::enable_if_t<std::is_integer_v<O>,int> = 0>
    explicit safe_int(O const &other) :
        value(safe_convert<T,OnOverflow>(other)) {}

    template<typename O, on_overflow_t OtherOnOverflow>
    explicit safe_int(safe_int<O,OtherOnOverflow> const &other) :
        value(safe_convert<T,OnOverflow>(other.value)) {}

    template<typename O, std::enable_if_t<std::is_integer_v<O>,int> = 0>
    explicit safe_int &operator=(O const &other) {
        value = safe_convert<T,OnOverflow>(other);
        return *this;
    }

    template<typename O, on_overflow_t OtherOnOverflow>
    explicit safe_int &operator=(safe_int<O,OtherOnOverflow> const &other) {
        value = safe_convert<T,OnOverflow>(other.value);
        return *this;
    }

    template<typename O, std::enable_if_t<std::is_integer_v<O>,int> = 0>
    explicit operator O () {
        return safe_convert<O,OnOverflow>(value);
    }
};

#define TEMPLATE(op)\
    template<typename T, on_overflow_t TO, typename U, on_overflow_t UO>\
    inline bool operator op (safe_int<T,TO> const &lhs, safe_int<U,UO> const &rhs) {\
        return lhs.value op rhs.value;\
    }\
    \
    template<typename T, on_overflow_t TO, typename U>\
    inline bool operator op (safe_int<T,TO> const &lhs, U const &rhs) {\
        return lhs.value op rhs;\
    }\
    template<typename T, typename U, on_overflow_t UO>\
    inline bool operator op (T const &lhs, safe_int<U,UO> const &rhs) {\
        return lhs op rhs.value;\
    }

TEMPLATE(==)
TEMPLATE(!=)
TEMPLATE(<)
TEMPLATE(>)
TEMPLATE(<=)
TEMPLATE(>=)
#undef TEMPLATE

#define TEMPLATE(op, func)\
    template<typename T, on_overflow_t TO, typename U, on_overflow_t UO>\
    safe_int<make_promote_t<T,U>,TO> operator op(safe_int<T,TO> const &lhs, safe_int<U,UO> const &rhs) {\
        return safe_int<make_promote_t<T,U>,TO>{ func<TO>(lhs.value, rhs.value) };\
    }\
    \
    template<typename T, on_overflow_t TO, typename U>\
    safe_int<make_promote_t<T,U>,TO> operator op(safe_int<T,TO> const &lhs, U const &rhs) {\
        return safe_int<make_promote_t<T,U>,TO>{ func<TO>(lhs.value, rhs) };\
    }\
    \
    template<typename T, typename U, on_overflow_t UO>\
    safe_int<make_promote_t<T,U>,TO> operator op(T const &lhs, safe_int<U,UO> const &rhs) {\
        return safe_int<make_promote_t<T,U>,TO>{ func<TO>(lhs, rhs.value) };\
    }

TEMPLATE(+, safe_add)
TEMPLATE(-, safe_sub)
TEMPLATE(*, safe_mul)
#undef TEMPLATE


}
