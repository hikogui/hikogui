// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "console.hpp"
#include "assert.hpp"
#include <Windows.h>
#include <debugapi.h>
#include <memory>

namespace tt {

void console_init() noexcept
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
        }

    } else {
        // stdout is already working, this happens when a UNIX-like shell
        // as setup stdin, stdout, stderr. For example when the application
        // is started from git-bash.
        // Since everything is already working, don't do anything.
        ;
    }
}

void console_output(std::string_view text, std::ostream &output) noexcept
{
    tt_assert(std::addressof(output) == std::addressof(std::cout) || std::addressof(output) == std::addressof(std::cerr));

    if (IsDebuggerPresent()) {
        ttlet text_ = to_wstring(text);
        OutputDebugStringW(text_.c_str());

    } else {
        output << text;
    }
}

} // namespace tt
