// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "os_detect.hpp"
#include "debugger.hpp"
#include <exception>

namespace tt {

#if TT_BUILD_TYPE == TT_BT_RELEASE
#define tt_no_default tt_unreachable();
#else
#define tt_no_default [[unlikely]] debugger_abort("tt_no_default");
#endif

#define tt_not_implemented [[unlikely]] debugger_abort("tt_not_implemented");
#define tt_overflow [[unlikely]] debugger_abort("overflow");

/** Assert if expression is true.
 * Independent of built type this macro will always check and abort on fail.
 * 
 */
#define tt_assert2(expression, msg) \
    do \
        { \
            if (!(expression)) \
                [[unlikely]] \
                { \
                    debugger_abort(msg); \
                } \
        } \
    while (false)

/** Assert if expression is true.
 * Independent of built type this macro will always check and abort on fail.
 */
#define tt_assert(expression) tt_assert2(expression, #expression)

} // namespace tt
