// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef ASSERT_HPP
#define ASSERT_HPP

#pragma once

#include "os_detect.hpp"
#include "debugger.hpp"
#include "utils.hpp"
#include <exception>

namespace tt {

/** Assert if expression is true.
 * Independent of built type this macro will always check and abort on fail.
 *
 */
#define tt_assert(expression, ...) \
    do { \
        if (!(expression)) { \
            if constexpr (__VA_OPT__(!) true) { \
                [[unlikely]] tt_debugger_abort(#expression); \
            } else { \
                [[unlikely]] tt_debugger_abort(__VA_ARGS__); \
            } \
        } \
    } while (false)

#if TT_BUILD_TYPE == TT_BT_DEBUG
/** Specify an axiom; an expression that is true.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 */
#define tt_axiom(expression, ...) tt_assert(expression __VA_OPT__(,) __VA_ARGS__)

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable else statements or switch-default labels,
 */
#define tt_no_default() [[unlikely]] tt_debugger_abort("tt_no_default()")

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
#define tt_static_no_default() []<bool Flag = false>(){ static_assert(Flag); }()

/** This part of the code has not been implemented yet.
 * This aborts the program.
 */
#define tt_not_implemented() [[unlikely]] tt_debugger_abort("tt_not_implemented()")

} // namespace tt

#endif
