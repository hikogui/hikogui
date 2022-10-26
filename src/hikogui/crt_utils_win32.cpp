// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "win32_headers.hpp"

#include "crt_utils.hpp"
#include "GUI/gui_system.hpp"
#include "subsystem.hpp"
#include "console.hpp"
#include "log.hpp"
#include "thread.hpp"
#include "exception.hpp"

namespace hi::inline v1 {

std::pair<int, char **> crt_start(int, char **, void *instance, int show_cmd)
{
    set_thread_name("main");

    // Switch out the terminate handler with one that can print an error message.
    old_terminate_handler = std::set_terminate(hi::terminate_handler);

    // lpCmdLine does not handle UTF-8 command line properly.
    // So use GetCommandLineW() to get wide string arguments.
    // CommandLineToArgW properly unescapes the command line
    // and splits in separate arguments.
    int wargc = 0;
    auto wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);

    // Convert the wchar arguments to UTF-8 and create nul terminated
    // c-strings. main() compatibility requires writable strings, so
    // we need to allocate old-style.
    char **argv = new char *[wargc + 2];
    int argc = 0;
    for (; argc != wargc; ++argc) {
        argv[argc] = hi::make_cstr(hi::to_string(std::wstring(wargv[argc])));
    }
    LocalFree(wargv);

    // Pass nShowCmd as a the second command line argument.
    if (show_cmd == 3) {
        argv[argc++] = hi::make_cstr("--window-state=maximize");
    } else if (show_cmd == 0 || show_cmd == 2 || show_cmd == 6 || show_cmd == 7 || show_cmd == 11) {
        argv[argc++] = hi::make_cstr("--window-state=minimize");
    }

    // Add a nullptr to the end of the argument list.
    argv[argc] = nullptr;

    // Initialize tzdata base.
    try {
        detail::log_message_base::zone = std::chrono::get_tzdb().current_zone();
    } catch (std::runtime_error const &e) {
        hi_log_error("Could not get current time zone: \"{}\"", e.what());
    }

    // Make sure the console is in a valid state to write text to it.
    hi::console_start();
    hi::time_stamp_count::start_subsystem();
    hi::start_system();

    hi::gui_system::instance = instance;
    return {argc, argv};
}

int crt_finish(int argc, char **argv, int exit_code)
{
    hi::shutdown_system();

    for (auto i = 0; i != argc; ++i) {
        delete[] argv[i];
    }
    delete[] argv;
    return exit_code;
}

} // namespace hi::inline v1
