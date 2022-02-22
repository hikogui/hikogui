// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "crt_utils.hpp"
#include "GUI/gui_system.hpp"
#include "URL.hpp"
#include "subsystem.hpp"
#include "console.hpp"
#include "log.hpp"
#include <Windows.h>

namespace tt::inline v1 {

static void configure_current_working_directory() noexcept
{
    DWORD required_buffer_size = GetCurrentDirectoryW(0, nullptr);
    if (!required_buffer_size) {
        tt_log_fatal("Could not get required buffer size.");
    }
    auto current_directory = std::make_unique<wchar_t[]>(required_buffer_size);
    if (GetCurrentDirectoryW(required_buffer_size, current_directory.get()) == 0) {
        tt_log_fatal("Could not get current directory: {}", get_last_error_message());
    }

    URL::setUrlForCurrentWorkingDirectory(URL::urlFromWPath(current_directory.get()));
}

std::pair<int, char **> crt_start(int, char **, void *instance, int show_cmd)
{
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
        argv[argc] = tt::make_cstr(tt::to_string(std::wstring(wargv[argc])));
    }
    LocalFree(wargv);

    // Pass nShowCmd as a the second command line argument.
    if (show_cmd == 3) {
        argv[argc++] = tt::make_cstr("--window-state=maximize");
    } else if (show_cmd == 0 || show_cmd == 2 || show_cmd == 6 || show_cmd == 7 || show_cmd == 11) {
        argv[argc++] = tt::make_cstr("--window-state=minimize");
    }

    // Add a nullptr to the end of the argument list.
    argv[argc] = nullptr;

    configure_current_working_directory();

    // Initialize tzdata base.
    try {
        detail::log_message_base::zone = std::chrono::get_tzdb().current_zone();
    } catch (std::runtime_error const &e) {
        tt_log_error("Could not get current time zone: \"{}\"", e.what());
    }

    // Make sure the console is in a valid state to write text to it.
    tt::console_start();
    tt::time_stamp_count::start_subsystem();
    tt::start_system();

    tt::gui_system::instance = instance;
    return {argc, argv};
}

int crt_finish(int argc, char **argv, int exit_code)
{
    tt::shutdown_system();

    for (auto i = 0; i != argc; ++i) {
        delete[] argv[i];
    }
    delete[] argv;
    return exit_code;
}

} // namespace tt::inline v1
