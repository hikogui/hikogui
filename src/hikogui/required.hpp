// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file required.hpp
 *
 * This file includes required definitions.
 */

#pragma once

#include <cstddef>
#include <string>
#include <chrono>

#ifndef hilet
/** Invariant should be the default for variables.
 *
 * C++ does have an invariant but it requires you to enter the 'const' keyword which
 * is easy to forget. Using a single keyword 'hilet' for an invariant makes it easier to notice
 * when you have defined a variant.
 */
#define hilet auto const
#endif

#ifndef hi_forward
/** Forward a value, based on the decltype of the value.
 */
#define hi_forward(x) std::forward<decltype(x)>(x)
#endif

// One clang-format off is not enough to stop clang-format to format.
// clang-format off
// clang-format off        
#define hi_num_va_args_impl( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define hi_num_va_args_(...) hi_num_va_args_impl(__VA_ARGS__)

/** Get the number of arguments.
 * 
 * @param ... A set of arguments
 * @return The number of arguments pass to this macro.
 */
#define hi_num_va_args(...) hi_num_va_args_(__VA_ARGS__ __VA_OPT__(,) \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0)
// clang-format on

#define hi_parans ()

#define hi_expand1(...) __VA_ARGS__
#define hi_expand2(...) hi_expand1(hi_expand1(hi_expand1(hi_expand1(__VA_ARGS__))))
#define hi_expand3(...) hi_expand2(hi_expand2(hi_expand2(hi_expand2(__VA_ARGS__))))
#define hi_expand4(...) hi_expand3(hi_expand3(hi_expand3(hi_expand3(__VA_ARGS__))))

/** Re-evaluate text up to 256 times by the pre-processor.
 *
 * @param ... The text which needs to be re-evaluated by the pre-processor.
 */
#define hi_expand(...) hi_expand4(hi_expand4(hi_expand4(hi_expand4(__VA_ARGS__))))

#define hi_for_each_again() hi_for_each_helper
#define hi_for_each_helper(macro, first_arg, ...) macro(first_arg) __VA_OPT__(hi_for_each_again hi_parans(macro, __VA_ARGS__))

/** Evaluate a macro for each argument.
 *
 * @param macro A macro that accepts a single argument.
 * @param ... A set of arguments to be passed one-by-one to @a macro.
 */
#define hi_for_each(macro, ...) __VA_OPT__(hi_expand(hi_for_each_helper(macro, __VA_ARGS__)))

namespace hi::inline v1 {

/** Signed size/index into an array.
 */
using ssize_t = std::ptrdiff_t;

#define ssizeof(x) (static_cast<ssize_t>(sizeof(x)))

constexpr std::size_t operator"" _uz(unsigned long long lhs) noexcept
{
    return static_cast<std::size_t>(lhs);
}

constexpr std::size_t operator"" _zu(unsigned long long lhs) noexcept
{
    return static_cast<std::size_t>(lhs);
}

constexpr std::ptrdiff_t operator"" _z(unsigned long long lhs) noexcept
{
    return static_cast<std::ptrdiff_t>(lhs);
}

#define hi_return_on_self_assignment(other) \
    if (&(other) == this) [[unlikely]] \
        return *this;

} // namespace hi::inline v1
