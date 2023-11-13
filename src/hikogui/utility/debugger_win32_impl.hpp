// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "../macros.hpp"
#include "console_win32.hpp"
#include "exception.hpp"
#include "debugger_intf.hpp"
#include <exception>
#include <print>
#include <cstdio>

hi_export_module(hikogui.utility.debugger : impl);

hi_warning_push();
// C6320: Exception-filter expression is the constant EXCEPTION_EXECUTE_HANDLER.
// This might mask exceptions that were not intended to be handled.
hi_warning_ignore_msvc(6320);

hi_export namespace hi {
inline namespace v1 {

namespace detail {

thread_local inline bool just_in_time_debugger_available = false;

}

hi_inline LONG launch_just_in_time_debugger(_EXCEPTION_POINTERS *p)
{
    // The UnhandledExceptionFilter will try to launch the just-in-time-debugger
    //  - EXCEPTION_CONTINUE_SEARCH:
    //    The just-in-time debugger was successfully launched by the user.
    //  - EXCEPTION_EXECUTE_HANDLER:
    //    The just-in-time-debugger was not launched by the user.
    //     - There are no debuggers available on the system.
    //     - No debuggers where configured as just-in-time-debuggers.
    //     - The user pressed 'Cancel' on the just-in-time-debugger
    //       dialogue.
    detail::just_in_time_debugger_available = UnhandledExceptionFilter(p) == EXCEPTION_CONTINUE_SEARCH;

#if HI_PROCESSOR == HI_CPU_X86_64
    // The breakpoint instruction is 0xCC (int 3), advance the instruction pointer.
    p->ContextRecord->Rip++;
#elif HI_PROCESSOR == HI_CPU_X86
    // The breakpoint instruction is 0xCC (int 3), advance the instruction pointer.
    p->ContextRecord->Eip++;
#else
#error "Not implemented."
#endif    

    // Continue at the saved instruction pointer.
    return EXCEPTION_CONTINUE_EXECUTION;
}

hi_export hi_no_inline hi_inline bool prepare_debug_break() noexcept
{
    // It is possible this function is called before main() and it will
    // need to make sure the console is started.
    start_console();

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
        // we will need to use a lower level API.
        // The AddVectoredExceptionHandler() adds an interrupt handler that
        // will catch the int 3 (debug break) instruction.
        //
        // Inside the function registered with AddVectoredExceptionHandler()
        // we will call UnhandledExceptionFilter() and store the result value
        // which will tell us if the debugger was launched by the user.
        //
        // UnhandledExceptionFilter() will present the user with a dialogue
        // to select one of the installed debuggers, or to cancel.
        hilet veh_handle = AddVectoredExceptionHandler(0, launch_just_in_time_debugger);

        // Attempt to break, which will interrupt.
        // This will eventually execute the UnhandledExceptionFilter(),
        // which may launch the just-in-time-debugger.
        DebugBreak();

        // Cleanup.
        RemoveVectoredExceptionHandler(veh_handle);

        // Return the result of the dialogue window presented to the user.
        return detail::just_in_time_debugger_available;
    }
}

} // namespace v1
} // namespace hi::inline v1

hi_warning_pop();
