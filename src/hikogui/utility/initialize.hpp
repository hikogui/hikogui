// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "terminate.hpp"
#include "console_win32.hpp"
#include "debugger_intf.hpp"

/**
 * 
 * @module: hikogui.utility.initialize
 */
hi_export_module(hikogui.utility.initialize)

hi_export namespace hi { inline namespace v1 {

/** Initialize base functionality of HikoGUI.
 * 
 * This will be called from `cpu_features_init()` which is started very early
 * before main().
 */
hi_inline void initialize() noexcept
{
    start_console();

    old_terminate_handler = std::set_terminate(hi::terminate_handler);

    setup_debug_break_handler();
}

}}