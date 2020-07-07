// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/os_detect.hpp"
#include "ttauri/debugger.hpp"
#include  <exception>

namespace tt {

#if TT_BUILD_TYPE == TT_BT_RELEASE
#define tt_no_default tt_unreachable();
#else
#define tt_no_default debugger_abort("tt_no_default");
#endif

#define tt_not_implemented debugger_abort("tt_not_implemented");
#define tt_overflow debugger_abort("overflow");

/** Assert if expression is true.
 * Independent of built type this macro will always check and abort on fail.
 */
#define tt_assert(expression)\
    do {\
        if (tt_unlikely(!(expression))) {\
            debugger_abort(# expression);\
        }\
    } while (false)

}
