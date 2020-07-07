// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/os_detect.hpp"

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "ttauri/GUI/VerticalSync_win32.hpp"
namespace tt {
using VerticalSync = VerticalSync_win32;
}

#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
#include "ttauri/GUI/VerticalSync_macos.hpp"
namespace tt {
using VerticalSync = VerticalSync_macos;
}

#else
#error "VerticalSync not implemented for os"
#endif
