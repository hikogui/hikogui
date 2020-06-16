// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/int_overflow.hpp"
#include "TTauri/Foundation/type_traits.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include <system_error>
#include <type_traits>
#include <limits>

namespace tt {

enum class on_overflow_t {
    //! On overflow throw an exception.
    Throw,
    //! On overflow saturate the result in the appropiate direction.
    Saturate,
    //! On overflow assert and terminate.
    Assert,
    //! On overflow assert and teminate in debug, assume in release.
    Axiom,
};

/*! Merge on_overflow_t.
 * Get the on_overflow of two arguments with highest safety.
 */
constexpr on_overflow_t operator|(on_overflow_t lhs, on_overflow_t rhs) noexcept {
    if (lhs == on_overflow_t::Throw || rhs == on_overflow_t::Throw) {
        return on_overflow_t::Throw;
    } else if (lhs == on_overflow_t::Saturate || rhs == on_overflow_t::Saturate) {
        return on_overflow_t::Saturate;
    } else if (lhs == on_overflow_t::Assert || rhs == on_overflow_t::Assert) {
        return on_overflow_t::Assert;
    } else {
        return on_overflow_t::Axiom;
    }
}

/*! Handle a potential overflow.
 * \param value returned by operation; potentially invalid.
 * \param overflow true when the operation overflowed.
 * \param is_positive true when saturation is at the positive limit. 
 */
template<typename T, on_overflow_t OnOverflow>
tt_force_inline T safe_handle_overflow(T value, bool overflow, bool is_positive) noexcept(OnOverflow != on_overflow_t::Throw)
{
    if constexpr (OnOverflow == on_overflow_t::Throw) {
        if (overflow) {
            TTAURI_THROW(math_error("safe_int"));
        }
    } else if constexpr (OnOverflow == on_overflow_t::Assert) {
        tt_assert(!overflow);
    } else if constexpr (OnOverflow == on_overflow_t::Axiom) {
        tt_assume(!overflow);
    } else if constexpr (OnOverflow == on_overflow_t::Saturate) {
        if (tt_unlikely(overflow)) {
            value = is_positive ? std::numeric_limits<T>::max() : std::numeric_limits<T>::min();
        }
    }
    return value;
}

template<typename T, on_overflow_t OnOverflow, typename U>
tt_force_inline T safe_convert(U const &rhs) noexcept(OnOverflow != on_overflow_t::Throw)
{
    T r;
    // Optimized away when is_same_v<T,U>
    ttlet overflow = convert_overflow(rhs, &r);
    return safe_handle_overflow<T,OnOverflow>(r, overflow, rhs >= 0);
}

template<on_overflow_t OnOverflow, typename T, typename U>
tt_force_inline make_promote_t<T,U> safe_add(T const &lhs, U const &rhs) noexcept(OnOverflow != on_overflow_t::Throw)
{
    make_promote_t<T,U> r;
    ttlet lhs_ = static_cast<make_promote_t<T,U>>(lhs);
    ttlet rhs_ = static_cast<make_promote_t<T,U>>(rhs);

    ttlet overflow = add_overflow(lhs_, rhs_, &r);
    return safe_handle_overflow<T,OnOverflow>(r, overflow, rhs_ >= 0);
}

template<on_overflow_t OnOverflow, typename T, typename U>
tt_force_inline make_promote_t<T,U> safe_sub(T const &lhs, U const &rhs) noexcept(OnOverflow != on_overflow_t::Throw)
{
    make_promote_t<T,U> r;
    ttlet lhs_ = static_cast<make_promote_t<T,U>>(lhs);
    ttlet rhs_ = static_cast<make_promote_t<T,U>>(rhs);

    ttlet overflow = sub_overflow(lhs_, rhs_, &r);
    return safe_handle_overflow<T,OnOverflow>(r, overflow, rhs_ >= 0);
}

template<on_overflow_t OnOverflow, typename T, typename U>
tt_force_inline make_promote_t<T,U> safe_mul(T const &lhs, U const &rhs) noexcept(OnOverflow != on_overflow_t::Throw)
{
    make_promote_t<T,U> r;
    ttlet lhs_ = static_cast<make_promote_t<T,U>>(lhs);
    ttlet rhs_ = static_cast<make_promote_t<T,U>>(rhs);

    ttlet overflow = mul_overflow(lhs_, rhs_, &r);
    return safe_handle_overflow<T,OnOverflow>(r, overflow, rhs_ >= 0);
}

template<typename T, on_overflow_t OnOverflow=on_overflow_t::Assert>
struct safe_int {
    using value_type = T;
    constexpr static on_overflow_t on_overflow = OnOverflow;

    T value;

    safe_int() : value(0) {}
    ~safe_int() = default;
    safe_int(safe_int const &) = default;
    safe_int(safe_int &&) = default;
    safe_int &operator=(safe_int const &) = default;
    safe_int &operator=(safe_int &&) = default;

    template<typename O, std::enable_if_t<std::is_integral_v<O>,int> = 0>
    explicit safe_int(O const &other) noexcept(OnOverflow != on_overflow_t::Throw) :
        value(safe_convert<T,OnOverflow>(other)) {}

    template<typename O, std::enable_if_t<std::is_floating_point_v<O>,int> = 0>
    explicit safe_int(O const &other) noexcept(OnOverflow != on_overflow_t::Throw) :
        value(safe_convert<T,OnOverflow>(other)) {}

    template<typename O, on_overflow_t OtherOnOverflow>
    explicit safe_int(safe_int<O,OtherOnOverflow> const &other) noexcept(OnOverflow != on_overflow_t::Throw) :
        value(safe_convert<T,OnOverflow>(other.value)) {}

    template<typename O, std::enable_if_t<std::is_integral_v<O>,int> = 0>
    safe_int &operator=(O const &other) noexcept(OnOverflow != on_overflow_t::Throw) {
        value = safe_convert<T,OnOverflow>(other);
        return *this;
    }

    template<typename O, on_overflow_t OtherOnOverflow>
    safe_int &operator=(safe_int<O,OtherOnOverflow> const &other) noexcept(OnOverflow != on_overflow_t::Throw) {
        value = safe_convert<T,OnOverflow>(other.value);
        return *this;
    }

    template<typename O, std::enable_if_t<std::is_integral_v<O>,int> = 0>
    explicit operator O () const noexcept(OnOverflow != on_overflow_t::Throw) {
        return safe_convert<O,OnOverflow>(value);
    }

    template<typename O, std::enable_if_t<std::is_floating_point_v<O>,int> = 0>
    explicit operator O () const noexcept {
        return static_cast<O>(value);
    }
};

#define TEMPLATE(op)\
    template<typename T, on_overflow_t TO, typename U, on_overflow_t UO>\
    tt_force_inline bool operator op (safe_int<T,TO> const &lhs, safe_int<U,UO> const &rhs) noexcept {\
        return lhs.value op rhs.value;\
    }\
    \
    template<typename T, on_overflow_t TO, typename U>\
    tt_force_inline bool operator op (safe_int<T,TO> const &lhs, U const &rhs) noexcept {\
        return lhs.value op rhs;\
    }\
    template<typename T, typename U, on_overflow_t UO>\
    tt_force_inline bool operator op (T const &lhs, safe_int<U,UO> const &rhs) noexcept {\
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
    tt_force_inline safe_int<make_promote_t<T,U>,TO|UO> operator op(safe_int<T,TO> const &lhs, safe_int<U,UO> const &rhs) noexcept((TO|UO) != on_overflow_t::Throw) {\
        return safe_int<make_promote_t<T,U>,TO|UO>{ func<TO|UO>(lhs.value, rhs.value) };\
    }\
    \
    template<typename T, on_overflow_t TO, typename U>\
    tt_force_inline safe_int<make_promote_t<T,U>,TO> operator op(safe_int<T,TO> const &lhs, U const &rhs) noexcept(TO != on_overflow_t::Throw) {\
        return safe_int<make_promote_t<T,U>,TO>{ func<TO>(lhs.value, rhs) };\
    }\
    \
    template<typename T, typename U, on_overflow_t UO>\
    tt_force_inline safe_int<make_promote_t<T,U>,UO> operator op(T const &lhs, safe_int<U,UO> const &rhs) noexcept(UO != on_overflow_t::Throw) {\
        return safe_int<make_promote_t<T,U>,UO>{ func<UO>(lhs, rhs.value) };\
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

using aint64_t = safe_int<int64_t,on_overflow_t::Assert>;
using aint32_t = safe_int<int32_t,on_overflow_t::Assert>;
using aint16_t = safe_int<int16_t,on_overflow_t::Assert>;
using aint8_t = safe_int<int8_t,on_overflow_t::Assert>;
using auint64_t = safe_int<uint64_t,on_overflow_t::Assert>;
using auint32_t = safe_int<uint32_t,on_overflow_t::Assert>;
using auint16_t = safe_int<uint16_t,on_overflow_t::Assert>;
using auint8_t = safe_int<uint8_t,on_overflow_t::Assert>;

using tint64_t = safe_int<int64_t,on_overflow_t::Throw>;
using tint32_t = safe_int<int32_t,on_overflow_t::Throw>;
using tint16_t = safe_int<int16_t,on_overflow_t::Throw>;
using tint8_t = safe_int<int8_t,on_overflow_t::Throw>;
using tuint64_t = safe_int<uint64_t,on_overflow_t::Throw>;
using tuint32_t = safe_int<uint32_t,on_overflow_t::Throw>;
using tuint16_t = safe_int<uint16_t,on_overflow_t::Throw>;
using tuint8_t = safe_int<uint8_t,on_overflow_t::Throw>;

using xint64_t = safe_int<int64_t,on_overflow_t::Axiom>;
using xint32_t = safe_int<int32_t,on_overflow_t::Axiom>;
using xint16_t = safe_int<int16_t,on_overflow_t::Axiom>;
using xint8_t = safe_int<int8_t,on_overflow_t::Axiom>;
using xuint64_t = safe_int<uint64_t,on_overflow_t::Axiom>;
using xuint32_t = safe_int<uint32_t,on_overflow_t::Axiom>;
using xuint16_t = safe_int<uint16_t,on_overflow_t::Axiom>;
using xuint8_t = safe_int<uint8_t,on_overflow_t::Axiom>;

}

namespace std {

template<> class numeric_limits<tt::safe_int<signed long long,tt::on_overflow_t::Saturate>>: public numeric_limits<signed long long> {};
template<> class numeric_limits<tt::safe_int<signed long,tt::on_overflow_t::Saturate>>: public numeric_limits<signed long> {};
template<> class numeric_limits<tt::safe_int<signed int,tt::on_overflow_t::Saturate>>: public numeric_limits<signed int> {};
template<> class numeric_limits<tt::safe_int<signed short,tt::on_overflow_t::Saturate>>: public numeric_limits<signed short> {};
template<> class numeric_limits<tt::safe_int<signed char,tt::on_overflow_t::Saturate>>: public numeric_limits<signed char> {};
template<> class numeric_limits<tt::safe_int<unsigned long long,tt::on_overflow_t::Saturate>>: public numeric_limits<unsigned long long> {};
template<> class numeric_limits<tt::safe_int<unsigned long,tt::on_overflow_t::Saturate>>: public numeric_limits<unsigned long> {};
template<> class numeric_limits<tt::safe_int<unsigned int,tt::on_overflow_t::Saturate>>: public numeric_limits<unsigned int> {};
template<> class numeric_limits<tt::safe_int<unsigned short,tt::on_overflow_t::Saturate>>: public numeric_limits<unsigned short> {};
template<> class numeric_limits<tt::safe_int<unsigned char,tt::on_overflow_t::Saturate>>: public numeric_limits<unsigned char> {};

template<> class numeric_limits<tt::safe_int<signed long long,tt::on_overflow_t::Assert>>: public numeric_limits<signed long long> {};
template<> class numeric_limits<tt::safe_int<signed long,tt::on_overflow_t::Assert>>: public numeric_limits<signed long> {};
template<> class numeric_limits<tt::safe_int<signed int,tt::on_overflow_t::Assert>>: public numeric_limits<signed int> {};
template<> class numeric_limits<tt::safe_int<signed short,tt::on_overflow_t::Assert>>: public numeric_limits<signed short> {};
template<> class numeric_limits<tt::safe_int<signed char,tt::on_overflow_t::Assert>>: public numeric_limits<signed char> {};
template<> class numeric_limits<tt::safe_int<unsigned long long,tt::on_overflow_t::Assert>>: public numeric_limits<unsigned long long> {};
template<> class numeric_limits<tt::safe_int<unsigned long,tt::on_overflow_t::Assert>>: public numeric_limits<unsigned long> {};
template<> class numeric_limits<tt::safe_int<unsigned int,tt::on_overflow_t::Assert>>: public numeric_limits<unsigned int> {};
template<> class numeric_limits<tt::safe_int<unsigned short,tt::on_overflow_t::Assert>>: public numeric_limits<unsigned short> {};
template<> class numeric_limits<tt::safe_int<unsigned char,tt::on_overflow_t::Assert>>: public numeric_limits<unsigned char> {};

template<> class numeric_limits<tt::safe_int<signed long long,tt::on_overflow_t::Throw>>: public numeric_limits<signed long long> {};
template<> class numeric_limits<tt::safe_int<signed long,tt::on_overflow_t::Throw>>: public numeric_limits<signed long> {};
template<> class numeric_limits<tt::safe_int<signed int,tt::on_overflow_t::Throw>>: public numeric_limits<signed int> {};
template<> class numeric_limits<tt::safe_int<signed short,tt::on_overflow_t::Throw>>: public numeric_limits<signed short> {};
template<> class numeric_limits<tt::safe_int<signed char,tt::on_overflow_t::Throw>>: public numeric_limits<signed char> {};
template<> class numeric_limits<tt::safe_int<unsigned long long,tt::on_overflow_t::Throw>>: public numeric_limits<unsigned long long> {};
template<> class numeric_limits<tt::safe_int<unsigned long,tt::on_overflow_t::Throw>>: public numeric_limits<unsigned long> {};
template<> class numeric_limits<tt::safe_int<unsigned int,tt::on_overflow_t::Throw>>: public numeric_limits<unsigned int> {};
template<> class numeric_limits<tt::safe_int<unsigned short,tt::on_overflow_t::Throw>>: public numeric_limits<unsigned short> {};
template<> class numeric_limits<tt::safe_int<unsigned char,tt::on_overflow_t::Throw>>: public numeric_limits<unsigned char> {};

template<> class numeric_limits<tt::safe_int<signed long long,tt::on_overflow_t::Axiom>>: public numeric_limits<signed long long> {};
template<> class numeric_limits<tt::safe_int<signed long,tt::on_overflow_t::Axiom>>: public numeric_limits<signed long> {};
template<> class numeric_limits<tt::safe_int<signed int,tt::on_overflow_t::Axiom>>: public numeric_limits<signed int> {};
template<> class numeric_limits<tt::safe_int<signed short,tt::on_overflow_t::Axiom>>: public numeric_limits<signed short> {};
template<> class numeric_limits<tt::safe_int<signed char,tt::on_overflow_t::Axiom>>: public numeric_limits<signed char> {};
template<> class numeric_limits<tt::safe_int<unsigned long long,tt::on_overflow_t::Axiom>>: public numeric_limits<unsigned long long> {};
template<> class numeric_limits<tt::safe_int<unsigned long,tt::on_overflow_t::Axiom>>: public numeric_limits<unsigned long> {};
template<> class numeric_limits<tt::safe_int<unsigned int,tt::on_overflow_t::Axiom>>: public numeric_limits<unsigned int> {};
template<> class numeric_limits<tt::safe_int<unsigned short,tt::on_overflow_t::Axiom>>: public numeric_limits<unsigned short> {};
template<> class numeric_limits<tt::safe_int<unsigned char,tt::on_overflow_t::Axiom>>: public numeric_limits<unsigned char> {};

}