// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "../macros.hpp"
#include <cstdio>

hi_export_module(hikogui.utility.console);

hi_export namespace hi {
inline namespace v1 {

/** Start the console.
 *
 * @return A runtime-depended boolean to make sure the function is started
 *         before main.
 */
hi_inline void start_console() noexcept
{
    auto out_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (out_handle == NULL) {
        // The stdout is not set, which means our parent process has
        // not set it. This is the most likely case on Windows 10.

        if (AttachConsole(ATTACH_PARENT_PROCESS)) {
            // Our parent process is a console, like cmd and powershell.
            // After attaching to the console we need re-open stdin,
            // stdout, stderr using the original device names.

            // Since stdin, stdout, stderr are macro's make sure we get pointers
            // which can be modified by freopen_s().
            FILE *fpstdin = stdin;
            FILE *fpstdout = stdout;
            FILE *fpstderr = stderr;

            freopen_s(&fpstdin, "CONIN$", "r", stdin);
            freopen_s(&fpstdout, "CONOUT$", "w", stdout);
            freopen_s(&fpstderr, "CONOUT$", "w", stderr);

            // Also set the win32 standard handles, so that this function can
            // be executed multiple times.
            auto stdin_handle = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
            SetStdHandle(STD_INPUT_HANDLE, stdin_handle);

            auto stdout_handle = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
            SetStdHandle(STD_OUTPUT_HANDLE, stdout_handle);
            SetStdHandle(STD_ERROR_HANDLE, stdout_handle);
        }

    } else {
        // stdout is already working, this happens when a UNIX-like shell
        // as setup stdin, stdout, stderr. For example when the application
        // is started from git-bash.
        // Since everything is already working, don't do anything.
    }
}

} // namespace v1
}