
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

#include "os_detect.hpp"

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include <Windows.h>
#endif

#pragma once

/** Main entry-point.
 *
 * @param arguments The command line arguments, split into tokens.
 *                  The first argument is the executable.
 * @param instance  An handle to the application instance.
 *                  On windows this is used open windows on this instance.
 */
int tt_main(std::vector<std::string> arguments, tt::os_handle instance);

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
int __clrcall WinMain(
    HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR lpCmdLine,
    int nShowCmd)
{
    int argc;
    auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc < 1) {
        std::cerr << "Missing executable from argument list." << std::endl;
        return 2;
    }

    auto arguments = std::vector<std::string>{};
    arguments.reserve(argc + 1);

    for (auto i = 0; i != argc; ++i) {
        arguments.push_back(std::move(to_string(std::wstring(argv[i]))));
    }
    LocalFree(argv);

    switch (nShowCmd) {
    case 3:
        argument.insert(std::next(std::begin(argument)), "--window-state=maximize");
        break;
    case 0:
    case 2:
    case 6:
    case 7:
    case 11:
        argument.insert(std::next(std::begin(argument)), "--window-state=minimize");
        break;
    default:;
    }

    return tt_main(arguments, hInstance);
}


#else

int main(int argc, char **argv)
{
    if (argc < 1) {
        std::cerr << "Missing executable from argument list." << std::endl;
        return 2;
    }

    auto arguments = std::vector<std::string>{};
    arguments.reserve(argc);

    for (int i = 0; i != argc; ++i) {
        arguments.emplace_back(argv[i]);
    }

    return tt_main(arguments, {});
}

#endif

