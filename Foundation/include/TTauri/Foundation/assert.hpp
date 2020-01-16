// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"
#include "TTauri/Foundation/debugger.hpp"
#include  <exception>

namespace TTauri {

#if defined(NDEBUG)
#define no_default ttauri_unreachable();
#else
#define no_default debugger_abort("no_default");
#endif

#define not_implemented debugger_abort("not_implemented");
#define ttauri_overflow debugger_abort("overflow");

/** Assert if expression is true.
 * Independent of NDEBUG macro this macro will always check and abort on fail.
 */
#define ttauri_assert(expression)\
    do {\
        if (ttauri_unlikely(!(expression))) {\
            debugger_abort(# expression);\
        }\
    } while (false)

}
