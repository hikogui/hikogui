// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file crt_utils.hpp Utilities for starting and stopping a hikogui application.
 */

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <tuple>

hi_export_module(hikogui.crt.crt_utils : intf);

hi_export namespace hi::inline v1 {

/** The application instance identified by the operating system.
*
* A handle to the application instance of the operating system:
*  - on windows this is the hInstance of WinMain(), and is used to create
*    windows that are owned by the current application.
*/
hi_inline os_handle crt_application_instance;

/** Start the hikogui system.
 *
 * This function will do a minimum amount of setup for the hikogui system.
 *
 * @param argc Number of arguments received from main().
 * @param argv The argument received from main().
 * @param instance A handle to the GUI instance of the operating system
 * @param show_cmd Information on how to open the first window.
 * @return argc, argv Normalized command line arguments in UTF-8 format. These
 *                    need to be passed to `crt_finish()` to free the memory.
 */
std::pair<int, char **> crt_start(int argc, char **argv, void *instance, int show_cmd);

/** Start the hikogui system.
 *
 * This function will do a minimum amount of setup for the hikogui system.
 *
 * @param instance A handle to the GUI instance of the operating system
 * @param show_cmd Information on how to open the first window.
 * @return argc, argv Normalized command line arguments in UTF-8 format.
 *                    These need to be passed to `crt_finish()` to free the memory.
 */
hi_inline std::pair<int, char **> crt_start(void *instance, int show_cmd)
{
    return crt_start(0, nullptr, instance, show_cmd);
}

/** Start the hikogui system.
 *
 * This function will do a minimum amount of setup for the hikogui system.
 *
 * @param argc Number of arguments received from main().
 * @param argv The argument received from main().
 * @return argc, argv Normalized command line arguments in UTF-8 format.
 *                    These need to be passed to `crt_finish()` to free the memory.
 */
hi_inline std::pair<int, char **> crt_start(int argc, char **argv)
{
    return crt_start(argc, argv, nullptr, 0);
}

/** Finish the hikogui system.
 *
 * This function will cleanly shutdown the hikogui system.
 *
 * @param argc The normalized number of arguments received from `crt_start()`.
 * @param argv The normalized arguments received from `crt_start()`.
 * @param exit_code The exit code of the application
 * @return The exit code of the application to return from `main()`.
 */
int crt_finish(int argc, char **argv, int exit_code);

} // namespace hi::inline v1
