// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file debugger.hpp Utilities to interact with the debugger this application runs under.
 */

#pragma once

#include "architecture.hpp"
#include "exception.hpp"
#include <format>

namespace hi { inline namespace v1 {

/** Prepare for breaking in the debugger.
 *
 * This will check if a debugger exists and potentially launch the Just-In-Time debugger if one is configured.
 * This function may terminate the application if no debugger is found.
 *
 * It does not do the actual breaking.
 */
void prepare_debug_break() noexcept;

#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS

/** Debug-break.
 *
 * This function will break the application in the debugger.
 * Potentially it will start the Just-In-Time debugger if one is configured.
 * Otherwise it will terminate the application and potentially dump a core file for post mortem debugging.
 */
#define hi_debug_break() \
    ::hi::prepare_debug_break(); \
    __debugbreak()

#else
#error Missing implementation of hi_debug_break().
#endif

/** Debug-break and abort the application.
 *
 * This function will break the application in the debugger.
 * Potentially it will start the Just-In-Time debugger if one is configured.
 *
 * Eventually it will terminate the application and potentially dump a core file for post mortem debugging.
 *
 * @param ... The reason why the abort is done.
 */
#define hi_debug_abort(...) \
    hi_set_terminate_message(__VA_ARGS__); \
    hi_debug_break(); \
    std::terminate()

}} // namespace hi::v1
