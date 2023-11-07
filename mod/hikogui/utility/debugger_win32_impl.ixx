// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"
#include "../win32_headers.hpp"

#include <exception>

export module hikogui_utility_debugger : impl;
import : intf;
import hikogui_utility_exception;

hi_warning_push();
// C6320: Exception-filter expression is the constant EXCEPTION_EXECUTE_HANDLER.
// This might mask exceptions that were not intended to be handled.
hi_warning_ignore_msvc(6320);

export namespace hi {
inline namespace v1 {

void launch_just_in_time_debugger(unsigned int v, _EXCEPTION_POINTERS *p)
{
    // We return the result of `UnhandledExceptionFilter() -> lONG` as an
    // exception.
    throw UnhandledExceptionFilter(p);
}

export hi_no_inline bool prepare_debug_break() noexcept
{
    if (IsDebuggerPresent()) {
        // When running under the debugger, __debugbreak() after returning.
        return true;

    } else {
        // If there is no debugger present we are going to try to launch the
        // just-in-time-debugger. This debugger is launched by the
        // UnhandledExceptionFilter(). However this function needs to be called
        // inside the __except(<expression>) filter expression.
        //
        // Since __try and __except are not available inside a C++20 module
        // we will need to use a lower level API. A se_translator is a function
        // that is called inside the filter-expression context and should normally
        // translate a structured exception to a C++ exception.
        //
        // So inside the se_translator function we will call
        // UnhandledExceptionFilter() and throw the result value which will
        // tell us if the debugger was launched by the user. A dialogue was
        // presented to the user for the selection of one of the installed
        // debuggers.

        auto old_handler = _set_se_translator(launch_just_in_time_debugger);

        try {
            // Attempt to break, causing an exception.
            // This will eventually execute the UnhandledExceptionFilter(),
            // which may launch the just-in-time-debugger.
            DebugBreak();

            // If we got here that means the debugger was attached to the
            // process between IsDebuggerPresent() and DebugBreak() calls.
            // It also means the debugger 'continue' over the break-point.
            // We return true, but the debugger will break again right after
            // this function ends.
            _set_se_translator(old_handler);
            return true;

        } catch (LONG e) {
            _set_se_translator(old_handler);

            switch (e) {
            case EXCEPTION_CONTINUE_SEARCH:
                // The just-in-time-debugger was not launched by the user.
                //  - There are no debuggers available on the system.
                //  - No debuggers where configured as just-in-time-debuggers.
                //  - The user pressed 'Cancel' on the just-in-time-debugger
                //    dialogue.
                return false;

            case EXCEPTION_EXECUTE_HANDLER:
                // The just-in-time debugger was successfully launched by the
                // user.
                return true;

            default:
                // UnhandledExceptionFilter() returned an unexpected value.
                std::terminate();
            }
        }
    }
}

} // namespace v1
} // namespace hi::inline v1

hi_warning_pop();
