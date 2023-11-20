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
#include "debugger_utils.hpp"
#include "misc.hpp"
#include <exception>
#include <print>
#include <cstdio>
#include <chrono>
#include <bit>
#include <format>
#include <thread>
#include <filesystem>

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

inline HANDLE jit_debug_handle = NULL;

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
    using namespace std::literals;

    auto debugger_enabled = win32_RegGetValue<std::string>(
        HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug", "Auto");
    if (not debugger_enabled or *debugger_enabled != "1") {
        // JIT debugger was not configured or disabled.
        return false;
    }

    auto debugger = win32_RegGetValue<std::string>(
        HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug", "Debugger");
    if (not debugger) {
        // JIT debugger was not configured.
        return false;
    }

    auto executable_name = win32_GetModuleFileName();
    if (not executable_name) {
        // Could not get executable name.
        return false;
    }

    auto executable_is_excluded = win32_RegGetValue<uint32_t>(
        HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug\\AutoExclusionList", executable_name->filename().string());
    if (executable_is_excluded and *executable_is_excluded == 1) {
        // This executable was excluded.
        return false;
    }

    hilet num_arguments = count(*debugger, "%ld") + count(*debugger, "%p");
    hilet cmd_line_fmt = replace(replace(*debugger, "%ld", "{}"), "%p", "{:x}");

    hilet process_id = GetCurrentProcessId();

    if (num_arguments >= 2 and jit_debug_handle == NULL) {
        if (auto handle = win32_CreateEvent()) {
            jit_debug_handle = *handle;

        } else {
            set_debug_message("Could not create event object for JIT debugger.");
            std::terminate();
        }
    }

    auto cmd_line = [&] {
        try {
            switch (num_arguments) {
            case 1:
                return std::vformat(cmd_line_fmt, std::make_format_args(process_id));
            case 2:
                return std::vformat(cmd_line_fmt, std::make_format_args(process_id, win32_HANDLE_to_int(jit_debug_handle)));
            case 3:
                return std::vformat(cmd_line_fmt, std::make_format_args(process_id, win32_HANDLE_to_int(jit_debug_handle), std::bit_cast<uintptr_t>(&jit_debug_info)));
            default:
                set_debug_message("JIT debugger accepts an invalid number of arguments.");
                std::terminate();
            }
        } catch (...) {
            std::terminate();
        }
    }();

    // Start debugger process
    auto startup_info = STARTUPINFOW{};
    startup_info.cb = sizeof(STARTUPINFOW);

    auto process_info = win32_CreateProcess(
        std::nullopt, // application name
        cmd_line, // command line
        nullptr, // process attributes
        nullptr, // thread attributes
        false, // inherit handles
        0, // creation flags
        nullptr, // environment
        std::nullopt, // current directory
        startup_info); // process info

    if (not process_info) {
        set_debug_message("Could not executed JIT debugger.");
        std::terminate();
    }

    hilet debugger_is_attached = [&] {
        auto end_time = std::chrono::utc_clock::time_point::max();
        while (std::chrono::utc_clock::now() < end_time) {
            if (IsDebuggerPresent()) {
                // The debugger is attached.
                return true;
            }

            // Block until the JIT debug selection application returns.
            if (end_time == std::chrono::utc_clock::time_point::max()) {
                auto exit_code = win32_GetExitCodeProcess(process_info->hProcess);

                if (exit_code and *exit_code == 0) {
                    // The user selected a debugger, we will wait upto 60 second.
                    end_time = std::chrono::utc_clock::now() + 60s;

                } else if (exit_code and *exit_code != 0) {
                    // User pressed "cancel".
                    return false;

                } else if (exit_code.error() != win32_error::status_pending) {
                    // The JIT debug process has not yet exited.
                    set_debug_message("GetExitCodeProcess() return unknown error");
                    std::terminate();
                }
            }

            std::this_thread::sleep_for(15ms);
        }

        set_debug_message("Debugger did not attach within 60s after being selected.");
        std::terminate();
    }();

    // Close handles created by CreateProcess.
    CloseHandle(process_info->hThread);
    CloseHandle(process_info->hProcess);

    return debugger_is_attached;
}

[[nodiscard]] constexpr std::string to_string(EXCEPTION_POINTERS const &ep) noexcept
{
    auto r = std::string{};

    // clang-format off
    switch (ep.ExceptionRecord->ExceptionCode) {
    case static_cast<DWORD>(STATUS_ASSERTION_FAILURE): r += "Assertion Failure"; break;
    case static_cast<DWORD>(EXCEPTION_ACCESS_VIOLATION): r += "Access Violation"; break;
    case static_cast<DWORD>(EXCEPTION_ARRAY_BOUNDS_EXCEEDED): r += "Array Bounds Exceeded"; break;
    case static_cast<DWORD>(EXCEPTION_BREAKPOINT): r += "Breakpoint"; break;
    case static_cast<DWORD>(EXCEPTION_DATATYPE_MISALIGNMENT): r += "Datatype Misalignment"; break;
    case static_cast<DWORD>(EXCEPTION_FLT_DENORMAL_OPERAND): r += "Floating Point Denormal Operand"; break;
    case static_cast<DWORD>(EXCEPTION_FLT_DIVIDE_BY_ZERO): r += "Floating Point Divide by Zero"; break;
    case static_cast<DWORD>(EXCEPTION_FLT_INEXACT_RESULT): r += "Floating Point Inexact Result"; break;
    case static_cast<DWORD>(EXCEPTION_FLT_INVALID_OPERATION): r += "Floating Point Invalid Operation"; break;
    case static_cast<DWORD>(EXCEPTION_FLT_OVERFLOW): r += "Floating Point Overflow"; break;
    case static_cast<DWORD>(EXCEPTION_FLT_STACK_CHECK): r += "Floating Point Stack Check"; break;
    case static_cast<DWORD>(EXCEPTION_FLT_UNDERFLOW): r += "Floating Point Underflow"; break;
    case static_cast<DWORD>(EXCEPTION_ILLEGAL_INSTRUCTION): r += "Illegal Instruction"; break;
    case static_cast<DWORD>(EXCEPTION_IN_PAGE_ERROR): r += "In Page Error"; break;
    case static_cast<DWORD>(EXCEPTION_INT_DIVIDE_BY_ZERO): r += "Integer Divide By Zero"; break;
    case static_cast<DWORD>(EXCEPTION_INT_OVERFLOW): r += "Integer Overflow"; break;
    case static_cast<DWORD>(EXCEPTION_INVALID_DISPOSITION): r += "Invalid Disposition"; break;
    case static_cast<DWORD>(EXCEPTION_NONCONTINUABLE_EXCEPTION): r += "Non-continuable Exception"; break;
    case static_cast<DWORD>(EXCEPTION_PRIV_INSTRUCTION): r += "Priviledged Instruction"; break;
    case static_cast<DWORD>(EXCEPTION_SINGLE_STEP): r += "Single Step"; break;
    case static_cast<DWORD>(EXCEPTION_STACK_OVERFLOW): r += "Stack Overflow"; break;
    default: r += "Unknown Operating System Exception"; break;
    }
    // clang-format on

    return r;
}

[[nodiscard]] constexpr bool is_debugable_exception(EXCEPTION_POINTERS const &ep) noexcept
{
    // clang-format off
    switch (ep.ExceptionRecord->ExceptionCode) {
    case static_cast<DWORD>(STATUS_ASSERTION_FAILURE): return true;
    case static_cast<DWORD>(EXCEPTION_ACCESS_VIOLATION): return true;
    case static_cast<DWORD>(EXCEPTION_ARRAY_BOUNDS_EXCEEDED): return true;
    case static_cast<DWORD>(EXCEPTION_BREAKPOINT): return true;
    case static_cast<DWORD>(EXCEPTION_DATATYPE_MISALIGNMENT): return true;
    case static_cast<DWORD>(EXCEPTION_FLT_DENORMAL_OPERAND): return true;
    case static_cast<DWORD>(EXCEPTION_FLT_DIVIDE_BY_ZERO): return true;
    case static_cast<DWORD>(EXCEPTION_FLT_INEXACT_RESULT): return true;
    case static_cast<DWORD>(EXCEPTION_FLT_INVALID_OPERATION): return true;
    case static_cast<DWORD>(EXCEPTION_FLT_OVERFLOW): return true;
    case static_cast<DWORD>(EXCEPTION_FLT_STACK_CHECK): return true;
    case static_cast<DWORD>(EXCEPTION_FLT_UNDERFLOW): return true;
    case static_cast<DWORD>(EXCEPTION_ILLEGAL_INSTRUCTION): return true;
    case static_cast<DWORD>(EXCEPTION_IN_PAGE_ERROR): return true;
    case static_cast<DWORD>(EXCEPTION_INT_DIVIDE_BY_ZERO): return true;
    case static_cast<DWORD>(EXCEPTION_INT_OVERFLOW): return true;
    case static_cast<DWORD>(EXCEPTION_INVALID_DISPOSITION): return true;
    case static_cast<DWORD>(EXCEPTION_NONCONTINUABLE_EXCEPTION): return true;
    case static_cast<DWORD>(EXCEPTION_PRIV_INSTRUCTION): return true;
    case static_cast<DWORD>(EXCEPTION_SINGLE_STEP): return false;
    case static_cast<DWORD>(EXCEPTION_STACK_OVERFLOW): return true;
    default: return false;
    }
    // clang-format on
}

hi_inline LONG exception_handler(EXCEPTION_POINTERS *p) noexcept
{
    if (not is_debugable_exception(*p)) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Fill in information about the exception so that the JIT debugger can handle it.
    jit_debug_info.dwSize = sizeof(JIT_DEBUG_INFO);
#if HI_PROCESSOR == HI_CPU_X86
    jit_debug_info.dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
#elif HI_PROCESSOR == HI_CPU_X86_64
    jit_debug_info.dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_AMD64;
#elif HI_PROCESSOR == HI_CPU_ARM
    jit_debug_info.dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_ARM;
#elif HI_PROCESSOR == HI_CPU_ARM64
    jit_debug_info.dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_ARM64;
#else
#error "Not implemented."
#endif
    jit_debug_info.dwThreadID = GetCurrentThreadId();
    jit_debug_info.dwReserved0 = 0;
    jit_debug_info.lpExceptionAddress = std::bit_cast<ULONG64>(p->ExceptionRecord->ExceptionAddress);
    jit_debug_info.lpExceptionRecord = std::bit_cast<ULONG64>(&p->ExceptionRecord);
    jit_debug_info.lpContextRecord = std::bit_cast<ULONG64>(p->ContextRecord);

    if (IsDebuggerPresent()) {
        // This is normally not reached.
        // But if the debugger is present, just do what normally is done.
        return EXCEPTION_CONTINUE_SEARCH;

    } else if (launch_jit_debugger()) {
        // The user selected a debugger.
        // The instruction that caused the exception will be executed again.

        // Clear the message set by a hi_assert_abort().
        ::hi::set_debug_message(nullptr);
        return EXCEPTION_CONTINUE_EXECUTION;

    } else if (p->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT) {
        // A break-point without a terminate message will simply continue.
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
        return EXCEPTION_CONTINUE_EXECUTION;

    } else {
        if (not (p->ExceptionRecord->ExceptionCode == STATUS_ASSERTION_FAILURE and has_debug_message())) {
            // The exception was not caused by a hi_assert_abort().
            auto exception_str = to_string(*p);
            set_debug_message(exception_str.c_str());
        }

        // If we reach this point we already tried opening the JIT debugger,
        // std::abort() should not.
        _set_abort_behavior(0, _CALL_REPORTFAULT);
        std::terminate();

        // A EXCEPTION_CONTINUE_SEARCH does not cause std::terminate() to be called.
    }
}

} // namespace detail

hi_inline void enable_debugger() noexcept
{
    // Disable error messages from the Windows CRT on std::terminate().
    _CrtSetReportMode(_CRT_WARN, 0);
    _CrtSetReportMode(_CRT_ERROR, 0);
    _CrtSetReportMode(_CRT_ASSERT, 0);

    // Install a handler for __debugbreak() / int 3 (0xCC).
    // This handler will request the user if it wants a debugger to be
    // attached to the application.
    AddVectoredExceptionHandler(0, detail::exception_handler);
}

} // namespace v1
} // namespace hi::inline v1

hi_warning_pop();
