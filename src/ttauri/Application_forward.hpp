// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "os_detect.hpp"

namespace tt {

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
class Application_win32;
using Application = Application_win32;

#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
class Application_macos;
using Application = Application_macos;

#else
#error "No implementation for forward Application."
#endif

inline Application *application = nullptr;

}


