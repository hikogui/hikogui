// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "architecture.hpp"
#include "type_traits.hpp"
#include "assert.hpp"

#include <type_traits>
#include <cmath>

#if HI_PROCESSOR == HI_CPU_X64
#include <immintrin.h>
#endif

#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include <intrin.h>
#pragma intrinsic(_mul128)
#endif

namespace hi::inline v1 {

template<typename T, typename U>
inline bool convert_overflow(T x, U *r)
{
    static_assert(std::is_integral_v<U>, "convert_overflow() requires integral return type.");
    static_assert(
        std::is_integral_v<T> || std::is_floating_point_v<T>, "convert_overflow() requires float or integral argument type.");

    if constexpr (std::is_integral_v<T>) {
        // Optimized away when is_same_v<T,U>
        *r = static_cast<U>(x);
        return *r != x;
    } else {
        *r = static_cast<U>(std::llround(x));
        return x < std::numeric_limits<U>::min() || x > std::numeric_limits<U>::max();
    }
}

template<typename T>
inline bool add_overflow(T lhs, T rhs, T *r)
{
    static_assert(std::is_integral_v<T>, "add_overflow() requires integral arguments.");

    if constexpr (compiler::current == compiler::gcc || compiler::current == compiler::clang) {
        // ADD, JO
        return __builtin_add_overflow(lhs, rhs, r);

    } else if constexpr (std::is_unsigned_v<T>) {
        // LEA, CMP, JB
        *r = lhs + rhs;
        return *r < lhs;

    } else {
        // LEA,XOR,XOR,TEST,JS
        hilet lhs_ = static_cast<std::make_unsigned_t<T>>(lhs);
        hilet rhs_ = static_cast<std::make_unsigned_t<T>>(rhs);
        hilet r_ = lhs_ + rhs_;
        *r = static_cast<T>(r_);
        return ((lhs ^ *r) & (rhs ^ *r)) < 0;
    }
}

template<typename T>
inline bool sub_overflow(T lhs, T rhs, T *r)
{
    static_assert(std::is_integral_v<T>, "sub_overflow() requires integral arguments.");

    if constexpr (compiler::current == compiler::gcc || compiler::current == compiler::clang) {
        // SUB, JB
        return __builtin_sub_overflow(lhs, rhs, r);

    } else if constexpr (std::is_unsigned_v<T>) {
        // MOV, SUB, CMP, JA
        *r = lhs - rhs;
        return *r > lhs;

    } else {
        // SUB, NOT, XOR, XOR, TEST, JL
        hilet lhs_ = static_cast<std::make_unsigned_t<T>>(lhs);
        hilet rhs_ = static_cast<std::make_unsigned_t<T>>(rhs);
        hilet r_ = lhs_ - rhs_;
        *r = static_cast<T>(r_);
        return ((lhs ^ rhs) & (~rhs ^ *r)) < 0;
    }
}

/** Multiply with overflow detection.
 * @return true when the multiplication overflowed.
 */
template<typename T>
inline bool mul_overflow(T lhs, T rhs, T *r) noexcept
{
    static_assert(std::is_integral_v<T>, "mul_overflow() requires integral arguments.");

    if constexpr (compiler::current == compiler::gcc || compiler::current == compiler::clang) {
        return __builtin_mul_overflow(lhs, rhs, r);

    } else if constexpr (std::is_signed_v<T> && compiler::current == compiler::msvc && sizeof(T) == sizeof(long long)) {
        // IMUL, SAR, XOR, JNE
        long long hi = 0;
        *r = _mul128(lhs, rhs, &hi);

        // Sign bit in *r should match all bits in hi.
        return (hi ^ (*r >> 63)) != 0;

    } else if constexpr (
        std::is_unsigned_v<T> && compiler::current == compiler::msvc && sizeof(T) == sizeof(unsigned long long)) {
        unsigned long long hi = 0;
        *r = _umul128(lhs, rhs, &hi);
        return hi > 0;

    } else if constexpr (sizeof(T) <= (sizeof(make_intmax_t<T>) / 2)) {
        // MOVSX, MOVSX, IMUL, MOVSX, CMP, JNE
        hilet lhs_ = static_cast<make_intmax_t<T>>(lhs);
        hilet rhs_ = static_cast<make_intmax_t<T>>(rhs);
        hilet r_ = lhs_ * rhs_;
        *r = static_cast<T>(r_);
        return *r != r_;

    } else {
        hi_not_implemented();
    }
}

} // namespace hi::inline v1