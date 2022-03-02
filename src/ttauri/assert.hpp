// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef ASSERT_HPP
#define ASSERT_HPP

#pragma once

#include "architecture.hpp"
#include "debugger.hpp"
#include "utils.hpp"
#include <exception>

namespace tt::inline v1 {

/** Assert if expression is true.
 * Independent of built type this macro will always check and abort on fail.
 *
 */
#define tt_assert(expression, ...) \
    do { \
        if (not(expression)) { \
            if constexpr (__VA_OPT__(not ) true) { \
                tt_debug_abort(#expression); \
            } else { \
                tt_debug_abort(__VA_ARGS__); \
            } \
        } \
    } while (false)

/** Assert if an expression is true.
 * If the expression is false then return from the function.
 *
 * @param x The expression to test
 * @param y The value to return from the current function.
 */
#define tt_assert_or_return(x, y) \
    if (!(x)) { \
        [[unlikely]] return y; \
    }

#if TT_BUILD_TYPE == TT_BT_DEBUG
/** Specify an axiom; an expression that is true.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 */
#define tt_axiom(expression, ...) tt_assert(expression __VA_OPT__(, ) __VA_ARGS__)

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable else statements or switch-default labels,
 */
#define tt_no_default() [[unlikely]] tt_debug_abort("tt_no_default()");

#else

/** Specify an axiom; an expression that is true.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 */
#define tt_axiom(expression, ...) tt_assume(expression)

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable else statements or switch-default labels,
 */
#define tt_no_default() tt_unreachable()
#endif

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable constexpr else statements.
 */
#define tt_static_no_default() \
    []<bool Flag = false>() \
    { \
        static_assert(Flag); \
    } \
    ()

/** This part of the code has not been implemented yet.
 * This aborts the program.
 */
#define tt_not_implemented() [[unlikely]] tt_debug_abort("tt_not_implemented()");

/** This part of the code has not been implemented yet.
 * This function should be used in unreachable constexpr else statements.
 */
#define tt_static_not_implemented() tt_static_no_default()

} // namespace tt::inline v1

#endif
