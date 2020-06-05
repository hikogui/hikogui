// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/os_detect.hpp"
#include "TTauri/Foundation/type_traits.hpp"
#include <type_traits>
#include <cmath>

#if PROCESSOR == CPU_X64
#include <immintrin.h>
#endif

#if OPERATING_SYSTEM == OS_WINDOWS
#include <intrin.h>
#pragma intrinsic(_mul128)
#endif

namespace TTauri {

template<typename T, typename U>
inline bool convert_overflow(T x, U *r)
{
    static_assert(std::is_integral_v<U>, "convert_overflow() requires integral return type.");
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "convert_overflow() requires float or integral argument type.");

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

    if constexpr (Compiler::current == Compiler::gcc || Compiler::current == Compiler::clang) {
        // ADD, JO
        return __builtin_add_overflow(lhs, rhs, r);

    } else if constexpr (std::is_unsigned_v<T>) {
        // LEA, CMP, JB
        *r = lhs + rhs;
        return *r < lhs;

    } else {
        // LEA,XOR,XOR,TEST,JS
        let lhs_ = static_cast<std::make_unsigned_t<T>>(lhs);
        let rhs_ = static_cast<std::make_unsigned_t<T>>(rhs);
        let r_ = lhs_ + rhs_;
        *r = static_cast<T>(r_);
        return ((lhs ^ *r) & (rhs ^ *r)) < 0;
    }
}

template<typename T>
inline bool sub_overflow(T lhs, T rhs, T *r)
{
    static_assert(std::is_integral_v<T>, "sub_overflow() requires integral arguments.");

    if constexpr (Compiler::current == Compiler::gcc || Compiler::current == Compiler::clang) {
        // SUB, JB
        return __builtin_sub_overflow(lhs, rhs, r);

    } else if constexpr (std::is_unsigned_v<T>) {
        // MOV, SUB, CMP, JA
        *r = lhs - rhs;
        return *r > lhs;

    } else {
        // SUB, NOT, XOR, XOR, TEST, JL
        let lhs_ = static_cast<std::make_unsigned_t<T>>(lhs);
        let rhs_ = static_cast<std::make_unsigned_t<T>>(rhs);
        let r_ = lhs_ - rhs_;
        *r = static_cast<T>(r_);
        return ((lhs ^ rhs) & (~rhs ^ *r)) < 0;
    }
}

/** Multiply with overflow detection.
 * @return true when the multiplication overflowed.
 */
template<typename T>
inline bool mul_overflow(T lhs, T rhs, T *r)
{
    static_assert(std::is_integral_v<T>, "mul_overflow() requires integral arguments.");

    if constexpr (Compiler::current == Compiler::gcc || Compiler::current == Compiler::clang) {
        return __builtin_mul_overflow(lhs, rhs, r);

    } else if constexpr (std::is_signed_v<T> && Compiler::current == Compiler::MSVC && sizeof(T) == sizeof(long long)) {
        // IMUL, SAR, XOR, JNE
        long long hi = 0;
        *r = _mul128(lhs, rhs, &hi);

        // Sign bit in *r should match all bits in hi.
        return (hi ^ (*r >> 63)) != 0;

    } else if constexpr (std::is_unsigned_v<T> && Compiler::current == Compiler::MSVC && sizeof(T) == sizeof(unsigned long long)) {
        unsigned long long hi = 0;
        *r = _umul128(lhs, rhs, &hi);
        return hi > 0; 

    } else if constexpr (sizeof(T) <= (sizeof(make_intmax_t<T>)/2)) {
        // MOVSX, MOVSX, IMUL, MOVSX, CMP, JNE
        let lhs_ = static_cast<make_intmax_t<T>>(lhs);
        let rhs_ = static_cast<make_intmax_t<T>>(rhs);
        let r_ = lhs_ * rhs_;
        *r = static_cast<T>(r_);
        return *r != r_;

    } else {
        not_implemented;
    }
}

}