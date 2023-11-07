// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file crt.hpp
 * CRT - Main entry point of a hikogui program.
 *
 * This header file will abstract the entry point for a program
 * for different operating systems, and call the `hi_main()` function
 * that should be defined as a portable entry point of the program.
 *
 * This header should be included only once by a only a single
 * translation-unit, as it defines `main()` or `WinMain()`.
 *
 * The work done by this abstraction is purposefully very limited,
 * its task it to make sure the command-line arguments are split into
 * tokens according to the rules of the operating system's shell. And
 * that the command line arguments are encoded as UTF-8.
 */

#pragma once

#include "macros.hpp"

hi_warning_push();
hi_warning_ignore_clang("-Wunused-variable");
hi_warning_ignore_clang("-Wreorder");
hi_warning_ignore_clang("-Wunused-private-field");

hi_warning_ignore_msvc(4127);
hi_warning_ignore_msvc(6011);
hi_warning_ignore_msvc(6386);
hi_warning_ignore_msvc(6387);
hi_warning_ignore_msvc(4701);
hi_warning_ignore_msvc(4703);
hi_warning_ignore_msvc(4189);

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

hi_warning_pop();

#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "win32_headers.hpp"

// Windows entry-point.
// This function will call `hi_main()`.
//
// It will use GetCommandLineW() to retrieve the command line in Unicode.
//
// The nShowCmd is used to insert add a command line argument at index 1:
//  - 1,4,5,8,9,10: (No command line argument added)
//  - 3: --window-state=maximize
//  - 0,2,6,7,11: --window-state=minimize
//

/** Main entry-point.
 *
 * @param ARGC argc type and name
 * @param ARGV argv type and name
 * @return Exit code.
 */
#define hi_main(ARGC, ARGV) \
    hi_main_detail(ARGC, ARGV); \
    int WINAPI WinMain( \
        _In_ HINSTANCE hInstance, \
        [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance, \
        [[maybe_unused]] _In_ LPSTR lpCmdLine, \
        _In_ int nShowCmd) \
    { \
        auto [argc, argv] = hi::crt_start(hInstance, nShowCmd); \
        hilet r = hi_main_detail(argc, argv); \
        return hi::crt_finish(argc, argv, r); \
    } \
    int hi_main_detail(ARGC, ARGV)

#else
#error "Need entry point for this architecture"
#endif
