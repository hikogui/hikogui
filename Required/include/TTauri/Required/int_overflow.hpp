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

template<typename T, std::enable_if<std::is_integral_v<T>, int> = 0>
inline bool add_overflow(T lhs, T rhs, T *r)
{
    if constexpr (compiler == Compiler::gcc || compiler == Compiler::clang) {
        return __builtin_add_overflow(lhs, rhs, r);

    } else if constexpr (std::is_unsigned_v<T> && processor == Processor::X64 && sizeof(T) == sizeof(uint64_t)) {
        return _addcarryx_u64(0, lhs, rhs, r);

    } else if constexpr (std::is_unsigned_v<T> && processor == Processor::X64 && sizeof(T) == sizeof(uint32_t)) {
        return _addcarryx_u32(0, lhs, rhs, r);

    } else if constexpr (std::is_unsigned_v<T>) {
        *r = lhs + rhs;
        return *r < lhs;

    } else if constexpr (sizeof(T) < sizeof(signed long long) {
        let lhs_ = static_cast<signed long long>(lhs);
        let rhs_ = static_cast<signed long long>(rhs);
        let r_ = lhs_ + rhs_;
        *r = static_cast<T>(r_);
        return *r != r_;

    } else {
        let lhs_ = static_cast<std::make_unsigned_t<T>>(lhs);
        let rhs_ = static_cast<std::make_unsigned_t<T>>(rhs);
        let r_ = lhs_ + rhs_;
        *r = static_cast<T>(r_);
        return ((lhs ^ *r) & (rhs ^ *r)) < 0;
    }
}

template<typename T, std::enable_if<std::is_integral_v<T>, int> = 0>
inline bool sub_overflow(T lhs, T rhs, T *r)
{
    if constexpr (compiler == Compiler::gcc || compiler == Compiler::clang) {
        return __builtin_sub_overflow(lhs, rhs, r);

    } else if constexpr (std::is_unsigned_v<T>) {
        *r = lhs - rhs;
        return *r > lhs;

    } else if constexpr (sizeof(T) < sizeof(signed long long) {
        let lhs_ = static_cast<signed long long>(lhs);
        let rhs_ = static_cast<signed long long>(rhs);
        let r_ = lhs_ - rhs_;
        *r = static_cast<T>(r_);
        return *r != r_;

    } else {
        let lhs_ = static_cast<std::make_unsigned_t<T>>(lhs);
        let rhs_ = static_cast<std::make_unsigned_t<T>>(rhs);
        let r_ = lhs_ - rhs_;
        *r = static_cast<T>(r_);
        //XXX test.
        return ((lhs ^ *r) & (rhs ^ *r)) < 0;
    }
}

template<typename T, std::enable_if<std::is_integral_v<T>, int> = 0>
inline bool mul_overflow(T lhs, T rhs, T *r)
{
    if constexpr (compiler == Compiler::gcc || compiler == Compiler::clang) {
        return __builtin_mul_overflow(lhs, rhs, r);

    } else if constexpr (std::is_signed_v<T> && compiler == Compiler::MSVC && sizeof(T) == sizeof(uint64_t)) {
        let hi = _mul128(lhs, rhs, r) > 0;
        return hi != 0 && hi != -1;

    } else if constexpr (std::is_unsigned_v<T> && processor == Processor::X64 && sizeof(T) == sizeof(uint64_t)) {
        return _mulx_u64(lhs, rhs, r) > 0;

    } else if constexpr (std::is_unsigned_v<T> && processor == Processor::X64 && sizeof(T) == sizeof(uint32_t)) {
        return _mulx_u32(lhs, rhs, r) > 0;

    } else if constexpr (std::is_unsigned_v<T> && sizeof(T) < sizeof(unsigned long long)) {
        let lhs_ = static_cast<unsigned long long>(lhs);
        let rhs_ = static_cast<unsigned long long>(rhs);
        let r_ = lhs_ * rhs_;
        *r = static_cast<T>(_r);
        return *r != r_;

    } else if constexpr (std::is_signed_v<T> && sizeof(T) < sizeof(signed long long)) {
        let lhs_ = static_cast<signed long long>(lhs);
        let rhs_ = static_cast<signed long long>(rhs);
        let r_ = lhs_ * rhs_;
        *r = static_cast<T>(_r);
        return *r != r_;

    } else {
        not_implemented;
    }
}

