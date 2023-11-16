// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file debugger.ixx Utilities to interact with the debugger this application runs under.
 */

#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.utility.debugger : intf);

hi_export namespace hi {
inline namespace v1 {

/** Enable the JIT debugger to be attached.
 * 
 * Normally the JIT debugger will already work. By using this function
 * hi_assert_break() and hi_debug_break() will improve.
 *  - hi_assert_break() will call std::terminate() for a better error message
 *    and stack-trace.
 *  - hi_debug_break() will continue if no debugger is available or cancelled. 
 */
void enable_debugger() noexcept;

} // namespace v1
} // namespace hi::v1
