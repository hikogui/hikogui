// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include "debugger.hpp"
#include "utils.hpp"
#include "utility.hpp"
#include <exception>

namespace hi::inline v1 {

/** Assert if expression is true.
 * Independent of built type this macro will always check and abort on fail.
 *
 * @param expression The expression to test.
 */
#define hi_assert(expression) \
    do { \
        if (not(expression)) { \
            hi_debug_abort(); \
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

#ifndef NDEBUG
/** Specify an axiom; an expression that is true.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 *
 * @param expression The expression that is true.
 */
#define hi_axiom(expression) hi_assert(expression)

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable else statements or switch-default labels,
 */
#define hi_no_default() [[unlikely]] hi_debug_abort()

#else
/** Specify an axiom; an expression that is true.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 *
 * @param expression The expression that is true.
 */
#define hi_axiom(expression) hi_assume(expression)

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable else statements or switch-default labels,
 */
#define hi_no_default() hi_unreachable()
#endif

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable constexpr else statements.
 */
#define hi_static_no_default() \
    []<bool Flag = false>() \
    { \
        static_assert(Flag); \
    } \
    ()

/** This part of the code has not been implemented yet.
 * This aborts the program.
 */
#define hi_not_implemented() [[unlikely]] hi_debug_abort();

/** This part of the code has not been implemented yet.
 * This function should be used in unreachable constexpr else statements.
 */
#define hi_static_not_implemented() hi_static_no_default()

} // namespace hi::inline v1
