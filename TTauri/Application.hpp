// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "os_detect.hpp"

#if OPERATING_SYSTEM == OS_WINDOWS

#include "Application_win32.hpp"
namespace TTauri {
using Application = Application_win32;
}

#elif OPERATING_SYSTEM == OS_MACOS

#include "Application_macos.hpp"
namespace TTauri {
using Application = Application_macos;
}

#else
#error "No Application implementation for this operating system."
#endif

namespace TTauri {
inline Application *application = nullptr;
}


