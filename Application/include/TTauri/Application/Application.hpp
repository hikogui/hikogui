// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"

#if OPERATING_SYSTEM == OS_WINDOWS

#include "TTauri/Application/Application_win32.hpp"
namespace TTauri {
using Application = Application_win32;
}

#define MAIN_ARGUMENTS main_hInstance, main_nCmdShow
#define MAIN_DEFINITION\
    int WINAPI wWinMain(_In_ HINSTANCE main_hInstance, _In_opt_ HINSTANCE main_hPrevInstance, _In_ PWSTR main_pCmdLine, _In_ int main_nCmdShow)


#elif OPERATING_SYSTEM == OS_MACOS

#include "TTauri/Application/Application_macos.hpp"
namespace TTauri {
using Application = Application_macos;
}

#define MAIN_ARGUMENTS argc, argv
#define MAIN_DEFINITION int main(int argc, char *argv[])

#else
#error "No Application implementation for this operating system."
#endif
