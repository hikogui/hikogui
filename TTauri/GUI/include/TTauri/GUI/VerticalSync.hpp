// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/os_detect.hpp"

#if OPERATING_SYSTEM == OS_WINDOWS
#include "VerticalSync_win32.hpp"
namespace TTauri::GUI {
using VerticalSync = VerticalSync_win32;
}

#elif OPERATING_SYSTEM == OS_MACOS
#include "VerticalSync_macos.hpp"
namespace TTauri::GUI {
using VerticalSync = VerticalSync_macos;
}

#else
#error "VerticalSync not implemented for os"
#endif
