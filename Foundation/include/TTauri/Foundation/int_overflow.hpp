// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/os_detect.hpp"
#include "TTauri/Foundation/type_traits.hpp"
#include <type_traits>

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
    static_assert(std::is_integral_v<T> && std::is_integral_v<U>, "convert_overflow() requires integral argument and return type.");

    *r = static_cast<U>(x);
    // Optimized away when is_same_v<T,U>
    return *r != x;
}

template<typename T>
inline bool add_overflow(T lhs, T rhs, T *r)
{
    static_assert(std::is_integral_v<T>, "add_overflow() requires integral arguments.");

    if constexpr (compiler == Compiler::gcc || compiler == Compiler::clang) {
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

    if constexpr (compiler == Compiler::gcc || compiler == Compiler::clang) {
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

template<typename T>
inline bool mul_overflow(T lhs, T rhs, T *r)
{
    static_assert(std::is_integral_v<T>, "mul_overflow() requires integral arguments.");

    if constexpr (compiler == Compiler::gcc || compiler == Compiler::clang) {
        return __builtin_mul_overflow(lhs, rhs, r);

    } else if constexpr (std::is_signed_v<T> && compiler == Compiler::MSVC && sizeof(T) == sizeof(int64_t)) {
        // IMUL, SAR, XOR, JNE
        long long hi;
        *r = _mul128(lhs, rhs, &hi);

        // Sign bit in *r should match all bits in hi.
        return (hi ^ (*r >> 63)) != 0;

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