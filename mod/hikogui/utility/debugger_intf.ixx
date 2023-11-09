// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file debugger.ixx Utilities to interact with the debugger this application runs under.
 */

module;
#include "../macros.hpp"

#include <format>
#include <iostream>

export module hikogui_utility_debugger : intf;

export namespace hi { inline namespace v1 {

/** Message to show when the application is terminated because of a debug_abort.
 */
std::atomic<char const *> debug_message = nullptr;

/** Prepare for breaking in the debugger.
 *
 * This will check if a debugger exists and potentially launch the Just-In-Time debugger if one is configured.
 * It does not do the actual breaking.
 * 
 * @return true if the debugger is attached.
 */
bool prepare_debug_break() noexcept;

/** Prepare for breaking in the debugger.
 * 
 * This will check if a debugger exists and potentially launch the Just-In-Time debugger if one is configured.
 * It does not do the actual breaking.
 * 
 * If no debugger is attached, then the application is terminated with the given message.
 * 
 * @param msg The message to print to the console and dialogue window.
 */
hi_no_inline void prepare_debug_break(char const *msg) noexcept
{
    if (not prepare_debug_break()) {
        std::cerr << std::format("Abnormal termination.\n{}\n", msg);
        debug_message.store(msg, std::memory_order::relaxed);
        std::terminate();
    }
}

}} // namespace hi::v1
