// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "os_detect.hpp"

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "application_win32.hpp"

#define tt_main_arguments tt_main_hInstance, tt_main_nCmdShow
#define tt_main_definition \
    int WINAPI wWinMain( \
        _In_ HINSTANCE tt_main_hInstance, \
        _In_opt_ HINSTANCE tt_main_hPrevInstance, \
        _In_ PWSTR tt_main_pCmdLine, \
        _In_ int tt_main_nCmdShow)

#define tt_application(...) tt::application_win32(__VA_ARGS__)

#elif TT_OPERATING_SYSTEM == TT_MACOS

#define tt_main_arguments tt_main_argc, tt_main_argv
#define tt_main_definition int main(int tt_main_argc, char *tt_main_argv[])

#define tt_application(...) tt::application_macos(__VA_ARGS__)

#else
#error "application_main not implemented for this operating system"
#endif
