// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"

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

template<std::integral T>
constexpr bool add_overflow(T lhs, T rhs, T *r) noexcept
{
    if (not std::constant_evaluated()) {
#if HI_COMPILER == HI_CC_GCC || HI_COMPPILER == HI_CC_CLANG
        // ADD, JO
        return __builtin_add_overflow(lhs, rhs, r);
#endif
    }

    if constexpr (std::is_unsigned_v<T>) {
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

template<std::integral T>
constexpr bool sub_overflow(T lhs, T rhs, T *r)
{
    if (not std::constant_evaluated()) {
#if HI_COMPILER == HI_CC_GCC || HI_COMPPILER == HI_CC_CLANG
        // SUB, JO
        return __builtin_sub_overflow(lhs, rhs, r);
#endif
    }

    if constexpr (std::is_unsigned_v<T>) {
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
template<std::unsigned_integral T>
constexpr bool mul_overflow(T lhs, T rhs, T *r) noexcept
{
    if (not std::constant_evaluated()) {
#if HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG
        // MUL, JO
        return __builtin_mul_overflow(lhs, rhs, r);

#elif HI_COMPILER == HI_CC_MSVC
        if constexpr (std::is_same_v<T, uint64_t>) {
            auto hi = uint64_t{};
            *r = _umul128(lhs, rhs, &hi);
            return hi > 0;
        }
#endif
    }

    if constexpr (sizeof(T) < sizeof(unsigned long long)) {
        hilet lhs_ = static_cast<unsigned long long>(lhs);
        hilet rhs_ = static_cast<unsigned long long>(rhs);
        auto r_ = lhs_ * rhs_;
        *r = static_cast<T>(r_);
        r_ >>= sizeof(T) * CHAR_BIT;
        // No overflow when 0.
        return r_ > 0;

    } else {
        constexpr auto max_width = sizeof(T) * CHAR_BIT;
        hilet width = std::bit_width(lhs) + std::bit_width(rhs);
        *r = lhs * rhs;

        return (width > max_width) or ((width == max_width) and (lhs > std::numeric_limits<T>::max / rhs));
    }
}

/** Multiply with overflow detection.
 * @return true when the multiplication overflowed.
 */
template<std::signed_integral T>
constexpr bool mul_overflow(T lhs, T rhs, T *r) noexcept
{
    if (not std::constant_evaluated()) {
#if HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG
        // MUL, JO
        return __builtin_mul_overflow(lhs, rhs, r);

#elif HI_COMPILER == HI_CC_MSVC
        if constexpr (std::is_same_v<T, int64_t>) {
            // IMUL, SAR, XOR, JNE
            auto hi = int64_t{};
            *r = _mul128(lhs, rhs, &hi);

            // Sign bit in *r should match all bits in hi.
            return (hi ^ (*r >> 63)) != 0;
        }
#endif
    }

    if constexpr (sizeof(T) < sizeof(long long)) {
        hilet lhs_ = static_cast<long long>(lhs);
        hilet rhs_ = static_cast<long long>(rhs);
        auto r_ = lhs_ * rhs_;
        *r = static_cast<T>(r_);
        r_ >>= sizeof(T) * CHAR_BIT - 1;
        // No overflow when 0 or -1.
        r_ += 1;
        return static_cast<unsigned long long>(r_) > 1;

    } else {
        auto lhs_u = static_cast<unsigned long long>(lhs);
        auto rhs_u = static_cast<unsigned long long>(rhs);

        // The following cases handle the only valid cases when one of the operants is `int_min`.
        if (lhs == 0 or rhs == 0) {
            *r = 0;
            return false;
        } else if (lhs_s == 1) {
            *r = rhs;
            return false;
        } else if (rhs_s == 1) {
            *r = lhs;
            return false;
        }

        // Count the number of significant bits of the result.
        auto lhs_a = lhs >= 0 ? lhs : -lhs;
        auto rhs_a = rhs >= 0 ? rhs : -rhs;
        auto width = std::bit_width(lhs_a) + std::bit_width(rhs_a);

        // Short-cutting reduces the chance that an expensive divide is needed for accurate overflow test.
        if (res_bits > value_bit or ((res_bits == value_bit and lhs_a > unsigned_int_max / rhs_a)) {
            // On overflow saturate.
            return {(lhs_s >= 0) == (rhs_s >= 0) ? int_max : int_min};
        }

        *r = {lhs_s * rhs_s};
        return false;
    }
}

template<std::integral T>
constexpr bool overflow_div(T lhs, T rhs, T *r)
{
    if constexpr (std::is_signed_v<T>) {
        if (lhs == std::numeric_limits<T>::min() and rhs == -1) {
            return true;
        }
    }

    if (lhs == 0) {
        return true;
    }

    *r = lhs / rhs;
    return false;
}



} // namespace hi::inline v1
