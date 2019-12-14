// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"
#include "TTauri/Foundation/debugger.hpp"
#include  <exception>

namespace TTauri {

#define no_default debugger_abort("no_default");
#define not_implemented debugger_abort("not_implemented");
#define ttauri_overflow debugger_abort("overflow");

/** Assert if expression is true.
 * Independed of NDEBUG macro this macro will always check and abort on fail.
 */
#define ttauri_assert(expression)\
    do {\
        if (ttauri_unlikely(!(expression))) {\
            debugger_abort(# expression);\
        }\
    } while (false)

#if defined(NDEBUG)

/** Assert if expression is true.
 * When NDEBUG is set then the expression is assumed to be true and optimizes code.
 * When NDEBUG is unset this macro will check and abort on fail.
 */
#define ttauri_axiom(expression)\
    do {\
        static_assert(sizeof(expression) == 1);\
        ttauri_assume(expression);\
    } while (false)

#else

/** Assert if expression is true.
 * When NDEBUG is set then the expression is assumed to be true and optimizes code.
 * When NDEBUG is unset this macro will check and abort on fail.
 */
#define ttauri_axiom(expression) ttauri_assert(expression)
#endif

}
