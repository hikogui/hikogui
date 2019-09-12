// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"

#if OPERATING_SYSTEM == OS_WINDOWS

#include "TTauri/Application_win32.hpp"
namespace TTauri {
using Application = Application_win32;
}

#elif OPERATING_SYSTEM == OS_MACOS

#include "TTauri/Application_macos.hpp"
namespace TTauri {
using Application = Application_macos;
}

#else
#error "No Application implementation for this operating system."
#endif

namespace TTauri {

inline Application &application()
{
    // We are sure that the Application is the only possible instance of _application.
    // But we can't use dynamic_cast<> as it will fail during destruction of the Application.
    auto *p = static_cast<Application *>(_application);
    required_assert(p != nullptr);
    return *p;
}

}


