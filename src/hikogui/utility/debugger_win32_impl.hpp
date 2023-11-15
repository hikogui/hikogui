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
#include "misc.hpp"
#include <exception>
#include <print>
#include <cstdio>
#include <chrono>

hi_export_module(hikogui.utility.debugger : impl);

hi_warning_push();
// C6320: Exception-filter expression is the constant EXCEPTION_EXECUTE_HANDLER.
// This might mask exceptions that were not intended to be handled.
hi_warning_ignore_msvc(6320);

hi_export namespace hi {
inline namespace v1 {
namespace detail {

struct JIT_DEBUG_INFO {
    DWORD dwSize;
    DWORD dwProcessorArchitecture;
    DWORD dwThreadID;
    DWORD dwReserved0;
    ULONG64 lpExceptionAddress;
    ULONG64 lpExceptionRecord;
    ULONG64 lpContextRecord;
};

inline JIT_DEBUG_INFO jit_debug_info = {};

/** Launch and attach the JIT debugger to this application.
 * 
 * This will check the registry key, for which debugger to launch:
 *  - HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AeDebug\Debugger
 * 
 * @retval true The debugger has been attached.
 * @retval false No JIT debugger configured, or the user pressed cancel.
*/
hi_inline bool launch_jit_debugger() noexcept
{
    using namespace = std::literals;

    auto debugger = win32_RegGetValue<std::string>(
        HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug", "Debugger");
    if (not debugger) {
        return false;
    }

    // XXX check AutoExclusionList.

    hilet num_arguments = count(debugger, "%ld") + count(debugger, "%p");;
    debugger = replace(debugger, "%ld", "{}");
    debugger = replace(debugger, "%p", "{:x}");

    auto process_id = GetCurrentProcessId();

    switch (num_arguments) {
    case 1:
        debugger = std::format(debugger, process_id);
        break;
    case 2:
        debugger = std::format(debugger, process_id, event_handle);
        break;
    case 3:
        debugger = std::format(debugger, process_id, event_handle, static_cast<uintptr_t>(std::addressof(jit_debug_info));
        break;
    default:
        std::println("Invalid JIT debugger '{}'", debugger);
        return false;
    }

    // Start debugger process
    auto process_info = PROCESS_INFORMATION{};
    auto startup_info = STARTUPINFOW{};
    startup_info.cb = sizeof(si);

    auto r = win32_CreateProcess(
        std::nullopt, // application name
        debugger, // command line
        nullptr, // process attributes
        nullptr, // thread attributes
        false, // inherit handles
        0, // creation flags
        nullptr, // environment
        std::nullopt, // current directory
        &startup_info, // startup info
        &process_info); // process info

    if (not r) {
        std::println("Could not execute JIT debugger '{}': {}", debugger, r.error());
        return false;
    }

    // Block until the JIT debug selection application returns.
    auto exit_code = GetExitCodeProcess(process_info.hProcess);

    // Close handles created by CreateProcess.
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);

    if (exit_code == 0) {
        // The user selected a debugger, it may take a bit of time before the debugger is attached.
        for (auto i = 0; i != 4000; ++i) {
            if (IsDebuggerPresent()) {
                return true;
            }
            std::this_thread::sleep_for(15ms);
        }

        std::println("The JIT debugger failed to attach within 60 seconds.");
        return false;

    } else {
        std::println("The JIT debugger was cancelled by the user.");
        return false;
    }
}


hi_inline LONG exception_handler(EXCEPTION_POINTERS *p)
{
    // Fill in information about the exception so that the JIT debugger can handle it.
    jit_debug_info.dwSize = sizeof(JIT_DEBUG_INFO);
#if HI_PROCESSOR == HI_CPU_X86
    jit_debug_info.dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
#elif HI_PROCESSOR = HI_CPU_X86_64
    jit_debug_info.dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_AMD64;
#elif HI_PROCESSOR = HI_CPU_ARM
    jit_debug_info.dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_ARM;
#elif HI_PROCESSOR = HI_CPU_ARM64
    jit_debug_info.dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_ARM64;
#else
#error "Not implemented."
#endif
    jit_debug_info.dwThreadID = GetCurrentThreadId();
    jit_debug_info.dwReserved0 = 0;
    jit_debug_info.lpExceptionAddress = std::bit_cast<ULONG64>(p->ExceptionRecord.ExceptionAddress);
    jit_debug_info.lpExceptionRecord = std::bit_cast<ULONG64>(&p->ExceptionRecord);
    jit_debug_info.lpContextRecord = std::bit_cast<ULONG64>(p->ContextRecord);

    std::println(stderr, "test2");
    if (not IsDebuggerPresent()) {
        if (launch_jit_debugger()) {
            // The user selected a debugger.
            // The instruction that caused the exception will be executed again.
            return EXCEPTION_CONTINUE_EXECUTION;

        } else if (p->ExceptionRecorder.ExceptionCode == EXCEPTION_BREAKPOINT) {
            if (has_terminate_message()) {
                // A terminate message means we will terminate.
                std::terminate();

            } else {
                // A break-point without a terminate message will simply continue.
                std::println(stderr, "test4");
                // No debugger attached, advance the instruction pointer to not get into a loop.
#if HI_PROCESSOR == HI_CPU_X86_64
                // The breakpoint instruction is 0xCC (int 3), advance the instruction pointer.
                p->ContextRecord->Rip++;
#elif HI_PROCESSOR == HI_CPU_X86
                // The breakpoint instruction is 0xCC (int 3), advance the instruction pointer.
                p->ContextRecord->Eip++;
#else
#error "Not implemented."
#endif

            }
        } else {
            // Create a message for this exception.
            set_terminate_message("Received exception.");
            std::terminate();
        }
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

}

hi_inline void setup_debug_break_handler() noexcept
{
    // Install a handler for __debugbreak() / int 3 (0xCC).
    // This handler will request the user if it wants a debugger to be
    // attached to the application.
    AddVectoredExceptionHandler(1, detail::exception_handler);
}

} // namespace v1
} // namespace hi::inline v1

hi_warning_pop();
