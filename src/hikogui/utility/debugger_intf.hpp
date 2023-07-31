// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file debugger.ixx Utilities to interact with the debugger this application runs under.
 */

#pragma once

#include "../macros.hpp"
#include <format>

hi_export_module(hikogui.utility.debugger : intf);

hi_export namespace hi { inline namespace v1 {

/** Prepare for breaking in the debugger.
 *
 * This will check if a debugger exists and potentially launch the Just-In-Time debugger if one is configured.
 * This function may terminate the application if no debugger is found.
 *
 * It does not do the actual breaking.
 */
void prepare_debug_break() noexcept;

}} // namespace hi::v1
