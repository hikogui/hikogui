// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "win32_headers.hpp"

#include "console.hpp"
#include "strings.hpp"
#include "assert.hpp"
#include <memory>
#include <string_view>

namespace hi::inline v1 {

void console_start() noexcept
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
    hi_assert(std::addressof(output) == std::addressof(std::cout) || std::addressof(output) == std::addressof(std::cerr));

    if (IsDebuggerPresent()) {
        hilet text_ = to_wstring(text);
        OutputDebugStringW(text_.c_str());

    } else {
        output << text;
    }
}

} // namespace hi::inline v1
