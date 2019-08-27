// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "os_detect.hpp"

namespace TTauri {

#if OPERATING_SYSTEM == OS_WINDOWS
class Application_win32;
using Application = Application_win32;

#elif OPERATING_SYSTEM == OS_MACOS
class Application_macos;
using Application = Application_macos;

#else
#error "No implementation for forward Application."
#endif
}


