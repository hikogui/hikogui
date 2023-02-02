// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file utility.hpp Utilities used by the HikoGUI library itself.
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

/** Return the result of an expression if the expression is valid.
 *
 * This macro uses a `requires {}` expression to determine if the expression is valid.
 *
 * @param expression The expression to evaluate if it is valid.
 */
#define hi_return_if_valid(expression) \
    do { \
        if constexpr (requires { expression; }) { \
            return expression; \
        } \
    } while (false)

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

#define hi_stringify_(x) #x
#define hi_stringify(x) hi_stringify_(x)

#define hi_cat_(a, b) a##b
#define hi_cat(a, b) hi_cat_(a, b)

#define hi_return_on_self_assignment(other) \
    if (&(other) == this) [[unlikely]] \
        return *this;

/** Get an overloaded macro for 1 or 2 arguments.
 *
 * This macro allows dispatching to other macros based on the number of arguments.
 * ```
 * #define foo1(a) bar(a)
 * #define foo2(a, b) bar(a, b)
 * #define foo(...) hi_get_overloaded_macro2(__VA_ARGS__, foo2, foo1)(__VA_ARGS__)
 * ```
 */
#define hi_get_overloaded_macro2(_1, _2, name, ...) name

#if defined(__clang__)
#define hi_unreachable() __builtin_unreachable()
#define hi_assume(condition) __builtin_assume(to_bool(condition))
#define hi_force_inline inline __attribute__((always_inline))
#define hi_no_inline __attribute__((noinline))
#define hi_restrict __restrict__
#define hi_warning_push() _Pragma("warning(push)")
#define hi_warning_pop() _Pragma("warning(push)")
#define hi_warning_ignore_msvc(code)
#define hi_warning_ignore_clang(a) _Pragma(hi_stringify(clang diagnostic ignored a))
#define hi_export
#define hi_typename typename
#define hi_constexpr

#elif defined(_MSC_BUILD)
#define hi_unreachable() __assume(0)
#define hi_assume(condition) __assume(condition)
#define hi_force_inline __forceinline
#define hi_no_inline __declspec(noinline)
#define hi_restrict __restrict
#define hi_warning_push() _Pragma("warning( push )")
#define hi_warning_pop() _Pragma("warning( pop )")
#define hi_msvc_pragma(a) _Pragma(a)
#define hi_warning_ignore_msvc(code) _Pragma(hi_stringify(warning(disable : code)))
#define hi_warning_ignore_clang(a)
#define hi_export __declspec(dllexport)
#define hi_typename
#define hi_constexpr constexpr

#elif defined(__GNUC__)
#define hi_unreachable() __builtin_unreachable()
#define hi_assume(condition) \
    do { \
        if (!(condition)) \
            hi_unreachable(); \
    } while (false)
#define hi_force_inline inline __attribute__((always_inline))
#define hi_no_inline __attribute__((noinline))
#define hi_restrict __restrict__
#define hi_warning_push() _Pragma("warning(push)")
#define hi_warning_pop() _Pragma("warning(pop)")
#define hi_msvc_pragma(a)
#define hi_warning_ignore_clang(a)
#define msvc_pragma(a)
#define hi_typename

#else
#define hi_unreachable() std::terminate()
#define hi_assume(condition) static_assert(sizeof(condition) == 1)
#define hi_force_inline inline
#define hi_no_inline
#define hi_restrict
#define hi_warning_push()
#define hi_warning_pop()
#define hi_msvc_pragma(a)
#define hi_warning_ignore_clang(a)
#define msvc_pragma(a)
#define hi_typename
#endif

hi_warning_push();
// C26472: Don't use static_cast for arithmetic conversions, Use brace initialization, gsl::narrow_cast or gsl::narrow (type.1).
// We do not have access to narrow_cast in this file.
hi_warning_ignore_msvc(26472);
// C26473: Don't cast between pointer types where the source type and the target type are the same (type.1).
// hi_forward need to specifically cast a value to a reference using static_cast.
hi_warning_ignore_msvc(26473);

namespace hi { inline namespace v1 {

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

/** Compare then store if there was a change.
 * @return true if a store was executed.
 */
template<typename T, typename U>
[[nodiscard]] bool compare_store(T& lhs, U&& rhs) noexcept
{
    if (lhs != rhs) {
        lhs = std::forward<U>(rhs);
        return true;
    } else {
        return false;
    }
}

/** Compare then store if there was a change.
 *
 * @note This atomic version does an lhs.exchange(rhs, std::memory_order_relaxed)
 * @return true if a store was executed.
 */
template<typename T, typename U>
[[nodiscard]] bool compare_store(std::atomic<T>& lhs, U&& rhs) noexcept
{
    return lhs.exchange(rhs, std::memory_order::relaxed) != rhs;
}

/** Tag used for special functions or constructions to do a override compared to another function of the same na,e
 */
struct override_t {};

/** A type that can not be constructed, copied, moved or destructed.
 */
struct unusable_t {
    unusable_t() = delete;
    ~unusable_t() = delete;
    unusable_t(unusable_t const&) = delete;
    unusable_t(unusable_t&&) = delete;
    unusable_t &operator=(unusable_t const&) = delete;
    unusable_t &operator=(unusable_t&&) = delete;
};


template<class T, class U>
[[nodiscard]] constexpr auto&& forward_like(U&& x) noexcept
{
    constexpr bool is_adding_const = std::is_const_v<std::remove_reference_t<T>>;
    if constexpr (std::is_lvalue_reference_v<T&&>) {
        if constexpr (is_adding_const) {
            return std::as_const(x);
        } else {
            return static_cast<U&>(x);
        }
    } else {
        if constexpr (is_adding_const) {
            return std::move(std::as_const(x));
        } else {
            return std::move(x);
        }
    }
}

}} // namespace hi::v1

hi_warning_pop();
