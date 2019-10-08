// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/int_overflow.hpp"
#include "TTauri/Required/type_traits.hpp"
#include <type_traits>
#include <limits>

namespace TTauri {

enum class on_overflow_t {
    //! On overflow throw an exception.
    Throw,
    //! On overflow assert and terminate.
    Assert,
    //! On overflow assert and teminate in debug, assume in release.
    Axiom,
    //! On overflow saturate the result in the appropiate direction.
    Saturate
};

template<typename T, on_overflow_t OnOverflow>
inline T safe_overflow(T value, bool overflow, char const *message, bool is_max)
{
    if constexpr (OnOverflow == on_overflow_t::Throw) {
        if (overflow) {
            TTAURI_THROW(overflow_error(message));
        }
    } else if constexpr (OnOverflow == on_overflow_t::Assert) {
        required_assert(!overflow);
    } else if constexpr (OnOverflow == on_overflow_t::Axiom) {
        axiom_assert(!overflow);
    } else if constexpr (OnOverflow == on_overflow_t::Saturate) {
        if (ttauri_unlikely(overflow)) {
            value = is_max ? std::numeric_limits<T>::max() : std::numeric_limits<T>::min();
        }
    }
    return value;
}

template<typename T, on_overflow_t OnOverflow, typename U>
inline T safe_convert(U const &rhs)
{
    T r;
    // Optimized away when is_same_v<T,U>
    let overflow = convert_overflow(rhs, &r);
    return safe_overflow<T,OnOverflow>(r, overflow, "safe_convert", rhs >= 0);
}

template<on_overflow_t OnOverflow, typename T, typename U>
inline make_promote_t<T,U> safe_add(T const &lhs, U const &rhs)
{
    make_promote_t<T,U> r;
    let lhs_ = static_cast<make_promote_t<T,U>>(lhs);
    let rhs_ = static_cast<make_promote_t<T,U>>(rhs);

    T r;
    let overflow = add_overflow(lhs, rhs, &r);
    return safe_overflow<T,OnOverflow>(r, overflow, "safe_add", rhs >= 0);
}

template<on_overflow_t OnOverflow, typename T, typename U>
inline make_promote_t<T,U> safe_sub(T const &lhs, U const &rhs)
{
    make_promote_t<T,U> r;
    let lhs_ = static_cast<make_promote_t<T,U>>(lhs);
    let rhs_ = static_cast<make_promote_t<T,U>>(rhs);

    T r;
    let overflow = sub_overflow(lhs, rhs, &r);
    return safe_overflow<T,OnOverflow>(r, overflow, "safe_sub", rhs >= 0);
}

template<on_overflow_t OnOverflow, typename T, typename U>
inline make_promote_t<T,U> safe_mul(T const &lhs, U const &rhs)
{
    make_promote_t<T,U> r;
    let lhs_ = static_cast<make_promote_t<T,U>>(lhs);
    let rhs_ = static_cast<make_promote_t<T,U>>(rhs);

    T r;
    let overflow = mul_overflow(lhs, rhs, &r);
    return safe_overflow<T,OnOverflow>(r, overflow, "safe_mul", rhs >= 0);
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

using sint64_t = safe_int<int64_t,on_overflow_t::Saturate>;
using sint32_t = safe_int<int32_t,on_overflow_t::Saturate>;
using sint16_t = safe_int<int16_t,on_overflow_t::Saturate>;
using sint8_t = safe_int<int8_t,on_overflow_t::Saturate>;
using suint64_t = safe_int<uint64_t,on_overflow_t::Saturate>;
using suint32_t = safe_int<uint32_t,on_overflow_t::Saturate>;
using suint16_t = safe_int<uint16_t,on_overflow_t::Saturate>;
using suint8_t = safe_int<uint8_t,on_overflow_t::Saturate>;

using tint64_t = safe_int<int64_t,on_overflow_t::Terminate>;
using tint32_t = safe_int<int32_t,on_overflow_t::Terminate>;
using tint16_t = safe_int<int16_t,on_overflow_t::Terminate>;
using tint8_t = safe_int<int8_t,on_overflow_t::Terminate>;
using tuint64_t = safe_int<uint64_t,on_overflow_t::Terminate>;
using tuint32_t = safe_int<uint32_t,on_overflow_t::Terminate>;
using tuint16_t = safe_int<uint16_t,on_overflow_t::Terminate>;
using tuint8_t = safe_int<uint8_t,on_overflow_t::Terminate>;

using eint64_t = safe_int<int64_t,on_overflow_t::Throw>;
using eint32_t = safe_int<int32_t,on_overflow_t::Throw>;
using eint16_t = safe_int<int16_t,on_overflow_t::Throw>;
using eint8_t = safe_int<int8_t,on_overflow_t::Throw>;
using euint64_t = safe_int<uint64_t,on_overflow_t::Throw>;
using euint32_t = safe_int<uint32_t,on_overflow_t::Throw>;
using euint16_t = safe_int<uint16_t,on_overflow_t::Throw>;
using euint8_t = safe_int<uint8_t,on_overflow_t::Throw>;

using fint64_t = safe_int<int64_t,on_overflow_t::Fast>;
using fint32_t = safe_int<int32_t,on_overflow_t::Fast>;
using fint16_t = safe_int<int16_t,on_overflow_t::Fast>;
using fint8_t = safe_int<int8_t,on_overflow_t::Fast>;
using fuint64_t = safe_int<uint64_t,on_overflow_t::Fast>;
using fuint32_t = safe_int<uint32_t,on_overflow_t::Fast>;
using fuint16_t = safe_int<uint16_t,on_overflow_t::Fast>;
using fuint8_t = safe_int<uint8_t,on_overflow_t::Fast>;

}
