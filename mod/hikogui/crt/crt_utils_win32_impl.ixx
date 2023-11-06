// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"
#include "../win32_headers.hpp"


#include <cstddef>
#include <memory>
#include <cstring>
#include <string>
#include <cstdio>
#include <exception>
#include <compare>
#include <string_view>
#include <format>
#include <type_traits>
#include <chrono>


export module hikogui_crt_crt_utils : impl;
import : intf;
import hikogui_char_maps;
import hikogui_concurrency;
import hikogui_console;
import hikogui_crt_terminate;
import hikogui_telemetry;
import hikogui_time;
import hikogui_utility;

hi_warning_push();
// C26400: Do not assign the result of an allocation or a function cal with an owner<T> return value to... (i11)
// For compatibility reasons we work with raw pointers here.
hi_warning_ignore_msvc(26400);

export namespace hi { inline namespace v1 {

/** Copy a std::string to new memory.
 * The caller will have to delete [] return value.
 */
export [[nodiscard]] char *make_cstr(char const *c_str, std::size_t size = -1) noexcept
{
    if (size == -1) {
        size = std::strlen(c_str);
    }

    auto r = new char[size + 1];
    std::memcpy(r, c_str, size + 1);
    return r;
}

/** Copy a std::string to new memory.
 * The caller will have to delete [] return value.
 */
export [[nodiscard]] char *make_cstr(std::string const& s) noexcept
{
    return make_cstr(s.c_str(), s.size());
}

export void console_start() noexcept
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

export std::pair<int, char **> crt_start(int, char **, void *instance, int show_cmd)
{
    // Switch out the terminate handler with one that can print an error message.
    old_terminate_handler = std::set_terminate(terminate_handler);

    // lpCmdLine does not handle UTF-8 command line properly.
    // So use GetCommandLineW() to get wide string arguments.
    // CommandLineToArgW properly unescapes the command line
    // and splits in separate arguments.
    int wargc = 0;
    auto wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    hi_assert_not_null(wargv);

    // Convert the wchar arguments to UTF-8 and create nul terminated
    // c-strings. main() compatibility requires writable strings, so
    // we need to allocate old-style.
    char **argv = new char *[wargc + 2];
    hi_assert_not_null(argv);

    int argc = 0;
    for (; argc != wargc; ++argc) {
        argv[argc] = make_cstr(to_string(std::wstring(wargv[argc])));
    }
    LocalFree(wargv);

    // Pass nShowCmd as a the second command line argument.
    if (show_cmd == 3) {
        argv[argc++] = make_cstr("--window-state=maximize");
    } else if (show_cmd == 0 || show_cmd == 2 || show_cmd == 6 || show_cmd == 7 || show_cmd == 11) {
        argv[argc++] = make_cstr("--window-state=minimize");
    }

    // Add a nullptr to the end of the argument list.
    argv[argc] = nullptr;

    // Make sure the console is in a valid state to write text to it.
    console_start();
    hilet [tsc_frequency, aux_is_cpu_id] = hi::time_stamp_count::start_subsystem();

    start_system();
    if (aux_is_cpu_id) {
        hi_log_info("The AUX value from the time-stamp-count is equal to the cpu-id.");
    }
    hi_log_info("The measured frequency of the TSC is {} Hz.", tsc_frequency);

    crt_application_instance = instance;
    return {argc, argv};
}

int crt_finish(int argc, char **argv, int exit_code)
{
    hi_assert_not_null(argv);

    shutdown_system();

    for (auto i = 0; i != argc; ++i) {
        delete[] argv[i];
    }
    delete[] argv;
    return exit_code;
}

}} // namespace hi::inline v1

hi_warning_pop();
