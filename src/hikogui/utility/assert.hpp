// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file assert.hpp Utilities to assert and bound check.
 */

#pragma once

#include "architecture.hpp"
#include "debugger.hpp"
#include "utility.hpp"
#include "type_traits.hpp"
#include "exception.hpp"
#include <exception>
#include <ranges>

hi_warning_push();
// "C26472: Don't use a static_cast for arithmetic", asserts use static_cast specifically for savety.
hi_warning_ignore_msvc(26472);

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

    hilet index_ = static_cast<value_type>(index);
    hilet upper_ = static_cast<value_type>(upper);
    return index_ < upper_;
}

/** Check if an index is between the lower (inclusive) and upper (exclusive).
 *
 * @note It is undefined behavior when @a upper is lower than @a lower.
 * @param index The index to check.
 * @param lower The lower bound.
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

/** Check if the expression is valid, or throw a parse_error.
* 
* This function is used to check if an expression is correct during the
* parsing of data.
* 
* @param expression The expression to check.
* @param message The message to set in the parse_error.
* @param ... Optional format parameters for the message.
* @throws parse_error When the expression yields false.
*/
#define hi_check(expression, message, ...) \
    do { \
        if (not(expression)) { \
            if constexpr (__VA_OPT__(not ) false) { \
                throw parse_error(std::format(message __VA_OPT__(, ) __VA_ARGS__)); \
            } else { \
                throw parse_error(message); \
            } \
        } \
    } while (false)

/** Assert if a value is within bounds, or throw a parse_error.
 *
 * This function is used to check if a value is within bounds
 * during parsing of data.
 *
 * Lower-bound is inclusive and Upper-bound is exclusive.
 *
 * @param x The value to check if it is within bounds.
 * @param ... One upper-bound; or a lower-bound and upper-bound.
 * @throws parse_error When the expression yields false.
 */
#define hi_check_bounds(x, ...) \
    do { \
        if (not ::hi::bound_check(x, __VA_ARGS__)) { \
            throw parse_error("assert bounds: " hi_stringify(x) " between " hi_stringify(__VA_ARGS__)); \
        } \
    } while (false)

/** Get a subspan, or throw a parse_error.
 *
 * This function is used to get a subspan of data with bounds
 * checks during parsing of data.
 *
 * @param span The span to take the subspan of.
 * @param offset The offset within the span to start the subspan.
 * @param ... Optional count for the number of elements in the subspan.
 *            When not specified the subspan is up to the end of the span.
 * @return A subspan.
 * @throws parse_error When the subspan does not fit the given span.
 */
#define hi_check_subspan(span, offset, ...) \
    [&](auto _hi_check_subspan_span, size_t _hi_check_subspan_offset, auto... _hi_check_subspan_count) { \
        if constexpr (sizeof...(_hi_check_subspan_count) == 0) { \
            if (_hi_check_subspan_offset < _hi_check_subspan_span.size()) { \
                return _hi_check_subspan_span.subspan(_hi_check_subspan_offset); \
            } \
        } else if constexpr (sizeof...(_hi_check_subspan_count) == 1) { \
            if (_hi_check_subspan_offset + wide_cast<size_t>(_hi_check_subspan_count...) <= _hi_check_subspan_span.size()) { \
                return _hi_check_subspan_span.subspan(_hi_check_subspan_offset, _hi_check_subspan_count...); \
            } \
        } \
        throw parse_error( \
            "assert bounds on: " hi_stringify(span) ".subspan(" hi_stringify(offset __VA_OPT__(", ") __VA_ARGS__) ")"); \
    }(span, offset __VA_OPT__(, ) __VA_ARGS__)

/** Get an element from a span, or throw a parse_error.
 *
 * This function is used to get an element of span with bounds
 * checks during parsing of data.
 *
 * @param span The span to take the subspan of.
 * @param index The index to the element in the span.
 * @return A reference to the element.
 * @throws parse_error When the index is not contained in the given span.
 */
#define hi_check_at(span, index) \
    [&](auto _hi_check_subspan_span, size_t _hi_check_subspan_index) { \
        if (_hi_check_subspan_index < _hi_check_subspan_span.size()) { \
            return _hi_check_subspan_span[_hi_check_subspan_index]; \
        } else { \
            throw parse_error("assert bounds on: " hi_stringify(span) "[" hi_stringify(index) "]"); \
        } \
    }(span, index)

#define hi_hresult_check(expression) \
    ([](HRESULT result) { \
        if (FAILED(result)) { \
            throw ::hi::io_error(std::format("Call to '{}' failed with {:08x}", #expression, result)); \
        } \
        return result; \
    }(expression))

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
#define hi_axiom_bounds(x, ...) hi_assert_bounds(x, __VA_ARGS__)

/** Assert if an expression is not nullptr.
 * If the expression is not a nullptr then return from the function.
 *
 * @param expression The expression to test
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

hi_warning_pop();
