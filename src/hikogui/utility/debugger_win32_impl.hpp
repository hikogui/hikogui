// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "../macros.hpp"
#include "../win32/win32.hpp"
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

enum class jit_debugger_state {
    idle,
    found_debugger,
    terminate
};

inline thread_local jit_debugger_state thread_jit_debugger_state = jit_debugger_state::idle;


/** Launch and attach the JIT debugger to this application.
 * 
 * This will check the registry key, for which debugger to launch:
 *  - HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AeDebug\Debugger
 * 
 * @retval true The debugger has been attached.
 * @retval false No JIT debugger configured, or the user pressed cancel.
*/
//hi_inline bool launch_jit_debugger() noexcept
//{
//// Get System directory, typically c:\windows\system32
//    std::wstring systemDir(MAX_PATH+1, '\0');
//    UINT nChars = GetSystemDirectoryW(&systemDir[0], systemDir.length());
//    if (nChars == 0) return false; // failed to get system directory
//    systemDir.resize(nChars);
//
//    // Get process ID and create the command line
//    DWORD pid = GetCurrentProcessId();
//    std::wostringstream s;
//    s << systemDir << L"\\vsjitdebugger.exe -p " << pid;
//    std::wstring cmdLine = s.str();
//
//    // Start debugger process
//    STARTUPINFOW si;
//    ZeroMemory(&si, sizeof(si));
//    si.cb = sizeof(si);
//
//    PROCESS_INFORMATION pi;
//    ZeroMemory(&pi, sizeof(pi));
//
//    if (!CreateProcessW(NULL, &cmdLine[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return false;
//
//    // Close debugger process handles to eliminate resource leak
//    CloseHandle(pi.hThread);
//    CloseHandle(pi.hProcess);
//
//    // Wait for the debugger to attach
//    while (!IsDebuggerPresent()) Sleep(100);
//
//    // Stop execution so the debugger can take over
//    DebugBreak();
//    return true;
//}


hi_inline LONG launch_jit_debugger(_EXCEPTION_POINTERS *p)
{
    std::println(stderr, "test0");
    if (p->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT) {
        std::println(stderr, "test1");
        // This handler is only for break-points.
        return EXCEPTION_CONTINUE_SEARCH;
    }

    std::println(stderr, "test2");
    if (not IsDebuggerPresent()) {
        // The UnhandledExceptionFilter will try to launch the just-in-time-debugger
        //  - EXCEPTION_CONTINUE_SEARCH:
        //    The just-in-time debugger was successfully launched by the user.
        //  - EXCEPTION_EXECUTE_HANDLER:
        //    The just-in-time-debugger was not launched by the user.
        //     - There are no debuggers available on the system.
        //     - No debuggers where configured as just-in-time-debuggers.
        //     - The user pressed 'Cancel' on the just-in-time-debugger
        //       dialogue.
        std::println(stderr, "test2.1");
        if (UnhandledExceptionFilter(p) == EXCEPTION_EXECUTE_HANDLER) {
            // The instruction pointer still points to the break-point,
            // continue executing will now cause that same break-point to be
            // handled by the debugger.
            // Note: this handler will not execute while the debugger is attached.
            std::println(stderr, "test2.1.1");
            return EXCEPTION_CONTINUE_SEARCH;
        } else {
            std::println(stderr, "test2.1.2");
            return EXCEPTION_CONTINUE_SEARCH;
        }

    } else {
        std::println(stderr, "test2.2");
        return EXCEPTION_CONTINUE_SEARCH;
    }

    
//        std::println(stderr, "test4");
//        // No debugger attached, advance the instruction pointer to not get into a loop.
//#if HI_PROCESSOR == HI_CPU_X86_64
//        // The breakpoint instruction is 0xCC (int 3), advance the instruction pointer.
//        p->ContextRecord->Rip++;
//#elif HI_PROCESSOR == HI_CPU_X86
//        // The breakpoint instruction is 0xCC (int 3), advance the instruction pointer.
//        p->ContextRecord->Eip++;
//#else
//#error "Not implemented."
//#endif
//    
//        // Terminate the application.
//        std::println(stderr, "test5");
//        std::terminate();
}

}

hi_inline void setup_debug_break_handler() noexcept
{
    // Install a handler for __debugbreak() / int 3 (0xCC).
    // This handler will request the user if it wants a debugger to be
    // attached to the application.
    //AddVectoredExceptionHandler(1, detail::launch_jit_debugger);
}

} // namespace v1
} // namespace hi::inline v1

hi_warning_pop();
