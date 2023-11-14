// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file debugger.ixx Utilities to interact with the debugger this application runs under.
 */

#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.utility.debugger : intf);

hi_export namespace hi { inline namespace v1 {

/** Setup the handler for break-points
 */
void setup_debug_break_handler() noexcept;

}} // namespace hi::v1
