// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "debugger.hpp"
#include "strings.hpp"
#include "log.hpp"
#include <Windows.h>
#include <debugapi.h>

namespace tt::inline v1 {

void prepare_debug_break() noexcept
{
    if (IsDebuggerPresent()) {
        // When running under the debugger, __debugbreak() after returning.
        return;

    } else {
        __try {
            __try {
                // Attempt to break, causing an exception.
                DebugBreak();

                // The UnhandledExceptionFilter() will be called to attempt to attach a debugger.
                //  * If the jit-debugger is not configured the user gets a error dialogue-box that
                //    with "Abort", "Retry (Debug)", "Ignore". The "Retry" option will only work
                //    when the application is already being debugged.
                //  * When the jit-debugger is configured the user gets a dialogue window which allows
                //    a selection of debuggers and a "OK (Debug)", "Cancel (aborts application)".

            } __except (UnhandledExceptionFilter(GetExceptionInformation())) {
                // The jit-debugger is not configured and the user pressed any of the buttons.
                std::terminate();
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            // User pressed "OK", debugger has been attached, __debugbreak() after return.
        }
    }
}


} // namespace tt::inline v1
