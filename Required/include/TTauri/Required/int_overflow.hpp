// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"
#include "TTauri/Required/os_detect.hpp"
#include "TTauri/Required/type_traits.hpp"
#include <type_traits>

#if PROCESSOR == CPU_X64
#include <immintrin.h>
#endif

#if OPERATING_SYSTEM == OS_WINDOWS
#include <intrin.h>
#pragma intrinsic(_mul128)
#endif

namespace TTauri {

template<typename T, typename U, std::enable_if_t<std::is_integral_v<T> && std::is_integral_v<U>,int> = 0>
inline bool convert_overflow(T x, U *r)
{
    *r = static_cast<U>(x);
    // Optimized away when is_same_v<T,U>
    return *r != x;
}

template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline bool add_overflow(T lhs, T rhs, T *r)
{
    if constexpr (compiler == Compiler::gcc || compiler == Compiler::clang) {
        // ADD, JO
        return __builtin_add_overflow(lhs, rhs, r);

    } else if constexpr (std::is_unsigned_v<T> && processor == Processor::X64 && sizeof(T) == sizeof(uint64_t)) {
        // ADD, JB
        uint64_t r_;
        let carry = _addcarryx_u64(0, static_cast<uint64_t>(lhs), static_cast<uint64_t>(rhs), &r_);
        *r = r_;
        return carry;

    } else if constexpr (std::is_unsigned_v<T> && processor == Processor::X64 && sizeof(T) == sizeof(uint32_t)) {
        // ADD, JB
        unsigned int r_;
        let carry = _addcarryx_u32(0, static_cast<unsigned int>(lhs), static_cast<unsigned int>(rhs), &r_);
        *r = r_;
        return carry;

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

template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline bool sub_overflow(T lhs, T rhs, T *r)
{
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

template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline bool mul_overflow(T lhs, T rhs, T *r)
{
    if constexpr (compiler == Compiler::gcc || compiler == Compiler::clang) {
        return __builtin_mul_overflow(lhs, rhs, r);

    } else if constexpr (std::is_signed_v<T> && compiler == Compiler::MSVC && sizeof(T) == sizeof(int64_t)) {
        // IMUL, SAR, XOR, JNE
        long long hi;
        *r = _mul128(lhs, rhs, &hi);

        // Sign bit in *r should match all bits in hi.
        return (hi ^ (*r >> 63)) != 0;

    } else if constexpr (std::is_unsigned_v<T> && processor == Processor::X64 && sizeof(T) == sizeof(uint64_t)) {
        // MULX, TEST, JNE
        uint64_t hi;
        *r = _mulx_u64(lhs, rhs, &hi);
        return hi > 0;

    } else if constexpr (std::is_unsigned_v<T> && processor == Processor::X64 && sizeof(T) == sizeof(uint32_t)) {
        // MULX, TEST, JNE
        unsigned int hi;
        *r = _mulx_u32(lhs, rhs, &hi);
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