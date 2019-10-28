// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"

#if OPERATING_SYSTEM == OS_WINDOWS

#include "TTauri/Application/Application_win32.hpp"
namespace TTauri {
using Application = Application_win32;
}

#define MAIN_ARGUMENTS hInstance, hPrevInstance, pCmdLine, nCmdShow
#define MAIN_DEFINITION\
    int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)


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
