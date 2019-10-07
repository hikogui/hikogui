// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"
#include "TTauri/Required/intrinsic.hpp"
#include "TTauri/Required/type_traits.hpp"
#include <limits>

namespace TTauri {

template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
T safe_add(T lhs, T rhs)
{
#elif COMPILER == CC_GCC || COMPILER == CC_CLANG
    T r;
    if (__builtin_add_overflow(lhs, rhs, &r)) {
        overflow;
    }
    return r;
#else
    let lhs_ = static_cast<std::to_unsigned_t<T>>(lhs);
    let rhs_ = static_cast<std::to_unsigned_t<T>>(rhs);

    let r_ = lhs_ + rhs_;
    let r = static_cast<T>(r_);

    // If the sign bits on the lhs and rhs are equal
    // and if the sign bit of the lhs and result lhs/rhs are different
    // then there is an overflow.
    if (((lhs ^ ~rhs) & (lhs ^ r)) < 0) {
        overflow;
    }

    return r;
#endif
}

template<typename T, typename U, std::enable_if<std::is_integral_v<T> && std::is_integral_v<U>, int> = 0>
auto safe_add(T lhs, U rhs) {
    return safe_add(static_cast<promote_t<T,U>>(lhs), static_cast<promote_t<T,U>>(rhs));
}

}
