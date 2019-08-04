// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"

#ifdef TTAURI_WINDOWS

#include "Application_win32.hpp"
namespace TTauri {
using Application = Application_win32;
}

#elif TTAURI_MACOS

#include "Application_macos.hpp"
namespace TTauri {
using Application = Application_macos;
}

#else
#error "No Application implementation for this operating system."
#endif
