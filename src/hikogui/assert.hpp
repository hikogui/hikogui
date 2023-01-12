// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include "debugger.hpp"
#include "utility.hpp"
#include "type_traits.hpp"
#include "exception.hpp"
#include <exception>
#include <ranges>

/** @file assert.hpp Utilities to assert and bound check.
 */

namespace hi { inline namespace v1 {

/** Check if an unsigned index is less than the bound.
 *
 * @param index The unsigned index to check.
 * @param upper The upper bound.
 * @return true If index is less than the upper bound.
 */
[[nodiscard]] constexpr bool bound_check(std::unsigned_integral auto index, std::unsigned_integral auto upper) noexcept
{
    using value_type = common_integer_t<decltype(index), decltype(upper)>;

    auto index_ = static_cast<value_type>(index);
    auto upper_ = static_cast<value_type>(upper);
    return index_ < upper_;
}

/** Check if an index is between the lower (inclusive) and upper (exclusive).
 *
 * @note It is undefined behavior when @a upper is lower than @a lower.
 * @param index The index to check.
 * @param upper The lower bound.
 * @param upper The upper bound.
 * @return true If index is greater or equal to lower bound and index is less than upper bound.
 */
[[nodiscard]] constexpr bool bound_check(std::integral auto index, std::integral auto lower, std::integral auto upper) noexcept
{
    using value_type = common_integer_t<decltype(index), decltype(lower), decltype(upper)>;

    auto index_ = static_cast<value_type>(index);
    auto lower_ = static_cast<value_type>(lower);
    auto upper_ = static_cast<value_type>(upper);

    hi_axiom(lower_ <= upper_);
    return index_ >= lower_ and index_ < upper_;
}

template<typename Context>
concept bound_check_range_helper = requires(Context&& range) {
                                       {
                                           std::ranges::size(range)
                                           } -> std::unsigned_integral;
                                   };

/** Check if an index is within a range.
 *
 * @param index The index to check.
 * @param range The range object, such as a vector or array, which has a `std::ranges::size()`.
 * @return true If index natural and below the size of the array.
 */
[[nodiscard]] constexpr bool bound_check(std::integral auto index, bound_check_range_helper auto&& range) noexcept
{
    static_assert(sizeof(index) <= sizeof(size_t));

    if constexpr (std::is_signed_v<std::decay_t<decltype(index)>>) {
        if (index < 0) {
            return false;
        }
    }

    return static_cast<size_t>(index) < std::ranges::size(range);
}

/** Assert if expression is true.
 * Independent of built type this macro will always check and abort on fail.
 *
 * @param expression The expression to test.
 * @param ... A string-literal explaining the reason why this assert exists.
 */
#define hi_assert(expression, ...) \
    do { \
        if (not(expression)) { \
            hi_debug_abort("assert: " __VA_ARGS__ " (" hi_stringify(expression) ")"); \
        } \
    } while (false)

/** Assert if an expression is true.
 * If the expression is false then return from the function.
 *
 * @param x The expression to test
 * @param y The value to return from the current function.
 */
#define hi_assert_or_return(x, y) \
    if (!(x)) { \
        [[unlikely]] return y; \
    }

/** Assert if a value is within bounds.
 * Independent of built type this macro will always check and abort on fail.
 *
 * Lower-bound is inclusive and Upper-bound is exclusive.
 * 
 * @param x The value to check if it is within bounds.
 * @param ... One upper-bound; or a lower-bound and upper-bound.
 */
#define hi_assert_bounds(x, ...) \
    do { \
        if (not ::hi::bound_check(x, __VA_ARGS__)) { \
            hi_debug_abort("assert bounds: " hi_stringify(x) " between " hi_stringify(__VA_ARGS__)); \
        } \
    } while (false)

/** Assert if an expression is not nullptr.
 * If the expression is not a nullptr then return from the function.
 *
 * @param x The expression to test
 * @param ... A string-literal as the reason why the not-null check exists.
 */
#define hi_assert_not_null(x, ...) \
    do { \
        if (x == nullptr) { \
            hi_debug_abort("assert not-null: " __VA_ARGS__ " (" hi_stringify(x) ")"); \
        } \
    } while (false)

#ifndef NDEBUG
/** Specify an axiom; an expression that is true.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 *
 * @param expression The expression that is true.
 * @param ... A string-literal as the reason why the axiom exists.
 */
#define hi_axiom(expression, ...) hi_assert(expression __VA_OPT__(, ) __VA_ARGS__)

/** Specify an axiom that the value is within bounds.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 *
 * Lower-bound is inclusive and Upper-bound is exclusive.
 *
 * @param x The value to check if it is within bounds.
 * @param ... One upper-bound; or a lower-bound and upper-bound.
 */
#define hi_axiom_bounds(a, ...) hi_assert_bounds(a, __VA_ARGS__)

/** Assert if an expression is not nullptr.
 * If the expression is not a nullptr then return from the function.
 *
 * @param x The expression to test
 * @param ... A string-literal as the reason why the not-null check exists.
 */
#define hi_axiom_not_null(expression, ...) hi_assert_not_null(expression __VA_OPT__(, ) __VA_ARGS__)

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable else statements or switch-default labels,
 *
 * @param ... A string-literal as the reason why the no-default exists.
 */
#define hi_no_default(...) [[unlikely]] hi_debug_abort("Reached no-default:" __VA_ARGS__)

#else
/** Specify an axiom; an expression that is true.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 *
 * @param expression The expression that is true.
 * @param ... A string-literal as the reason why the axiom exists.
 */
#define hi_axiom(expression, ...) hi_assume(expression)

/** Specify an axiom that the value is within bounds.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 *
 * Lower-bound is inclusive and Upper-bound is exclusive.
 *
 * @param x The value to check if it is within bounds.
 * @param ... One upper-bound; or a lower-bound and upper-bound.
 */
#define hi_axiom_bounds(x, ...) hi_assume(not ::hi::bound_check(x, __VA_ARGS__))

/** Assert if an expression is not nullptr.
 * If the expression is not a nullptr then return from the function.
 *
 * @param x The expression to test
 * @param ... A string-literal as the reason why the not-null check exists.
 */
#define hi_axiom_not_null(expression, ...) hi_assume(expression != nullptr)

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable else statements or switch-default labels,
 *
 * @param ... A string-literal as the reason why the no-default exists.
 */
#define hi_no_default(...) hi_unreachable()
#endif

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable constexpr else statements.
 *
 * @param ... A string-literal as the reason why the no-default exists.
 */
#define hi_static_no_default(...) \
    []<bool Flag = false>() \
    { \
        static_assert(Flag, "No default: " __VA_ARGS__); \
    } \
    ()

/** This part of the code has not been implemented yet.
 * This aborts the program.
 *
 * @param ... A string-literal as the reason why this it not implemented.
 */
#define hi_not_implemented(...) [[unlikely]] hi_debug_abort("Not implemented: " __VA_ARGS__);

/** This part of the code has not been implemented yet.
 * This function should be used in unreachable constexpr else statements.
 *
 * @param ... A string-literal as the reason why this it not implemented.
 */
#define hi_static_not_implemented(...) hi_static_no_default("Not implemented: " __VA_ARGS__)

}} // namespace hi::v1
