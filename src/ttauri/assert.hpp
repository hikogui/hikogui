// Copyright 2019, 2020 Pokitec
// All rights reserved.

#ifndef ASSERT_HPP
#define ASSERT_HPP

#pragma once

#include "os_detect.hpp"
#include "debugger.hpp"
#include "utils.hpp"
#include <exception>

namespace tt {

#if TT_BUILD_TYPE == TT_BT_RELEASE
#define tt_no_default() tt_unreachable()
#else
#define tt_no_default() [[unlikely]] tt_debugger_abort("tt_no_default()")
#endif

#define tt_not_implemented() [[unlikely]] tt_debugger_abort("tt_not_implemented()")

/** Assert if expression is true.
 * Independent of built type this macro will always check and abort on fail.
 *
 */
#define tt_assert(expression, ...) \
    do { \
        if (!(expression)) { \
            if constexpr (::tt::nr_arguments(__VA_ARGS__) == 0) { \
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
#define tt_axiom(expression, ...) tt_assert(expression, __VA_ARGS__)
#else
/** Specify an axiom; an expression that is true.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 */
#define tt_axiom(expression, ...) tt_assume(expression)
#endif

} // namespace tt

#endif