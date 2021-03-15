// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file crt.hpp
 * CRT - Main entry point of a ttauri program.
 *
 * This header file will abstract the entry point for a program
 * for different operating systems, and call the `tt_main()` function
 * that should be defined as a portable entry point of the program.
 *
 * This header should be included only once by a only a single
 * translation-unit, as it defines `main()` or `WinMain()`.
 *
 * The work done by this abstraction is purposfully very limitted,
 * its task it to make sure the command-line arguments are split into
 * tokens according to the rules of the operating system's shell. And
 * that the command line arguments are encoded as UTF-8.
 */

#pragma once

#include "os_detect.hpp"
#include "system_status.hpp"
#include "URL.hpp"
#include "strings.hpp"
#include "cast.hpp"

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "application_win32.hpp"
#define tt_application(...) tt::application_win32(__VA_ARGS__)

#include <Windows.h>
#elif TT_OPERATING_SYSTEM == TT_OS_MACOS
#include "application_macos.hpp"
#define tt_application(...) tt::application_macos(__VA_ARGS__)

#endif

#include <date/tz.h>

/** Main entry-point.
 *
 * @param argc Number of arguments
 * @param argv A nullptr terminated list of pointers to null terminated strings.
 * @param instance An handle to the application instance.
 *                 On windows this is used to open windows on this instance.
 * @return Exit code.
 */
int tt_main(int argc, char *argv[], tt::os_handle instance);

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS

/** Windows entry-point.
 * This function will call `tt_main()`.
 *
 * It will use GetCommandLineW() to retrieve the command line in Unicode.
 *
 * The nShowCmd is used to insert add a command line argument at index 1:
 *  - 1,4,5,8,9,10: (No command line argument added)
 *  - 3: --window-state=maximize
 *  - 0,2,6,7,11: --window-state=minimize
 */
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance,
    [[maybe_unused]] _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
    // lpCmdLine does not handle UTF-8 command line properly.
    // So use GetCommandLineW() to get wide string arguments.
    // CommandLineToArgW properly unescapes the command line
    // and splits in separate arguments.
    int argc;
    auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    // Convert the wchar arguments to UTF-8 and create nul terminated
    // c-strings. main() compatibility requires writable strings, so
    // we need to allocate old-style.
    auto arguments = std::vector<char *>{};
    arguments.reserve(argc + 2);
    for (auto i = 0; i != argc; ++i) {
        arguments.push_back(tt::make_cstr(tt::to_string(std::wstring(argv[i]))));
    }
    LocalFree(argv);

    // Pass nShowCmd as a the second command line argument.
    if (nShowCmd == 3) {
        arguments.insert(std::next(std::begin(arguments)), tt::make_cstr("--window-state=maximize"));
    } else if (nShowCmd == 0 || nShowCmd == 2 || nShowCmd == 6 || nShowCmd == 7 || nShowCmd == 11) {
        arguments.insert(std::next(std::begin(arguments)), tt::make_cstr("--window-state=minimize"));
    }

    // Add a nullptr to the end of the argument list.
    arguments.push_back(nullptr);

    // Initialize tzdata base.
#if USE_OS_TZDB == 0
    ttlet tzdata_location = tt::URL::urlFromResourceDirectory() / "tzdata";
    date::set_install(tzdata_location.nativePath());
    try {
        [[maybe_unused]] ttlet time_zone = date::current_zone();
    } catch (std::runtime_error const &e) {
        tt_log_error("Could not get current time zone: \"{}\"", e.what());
    }
#endif

    ttlet r = tt_main(tt::narrow_cast<int>(arguments.size() - 1), arguments.data(), hInstance);

    tt::system_status_shutdown();

    for (auto argument: arguments) {
        delete [] argument;
    }
    return r;
}

#else

int main(int argc, char *argv[])
{
    // XXX - The URL system needs to know about the location of the executable.
#if USE_OS_TZDB == 0
    ttlet tzdata_location = tt::URL::urlFromResourceDirectory() / "tzdata";
    date::set_install(tzdata_location.nativePath());
    try {
        [[maybe_unused]] ttlet time_zone = date::current_zone();
    } catch (std::runtime_error const &e) {
        tt_log_error("Could not get current time zone: \"{}\"", e.what());
    }
#endif

    ttlet r = tt_main(argc, argv, {});
    tt::system_status_shutdown();
    return r;
}

#endif

