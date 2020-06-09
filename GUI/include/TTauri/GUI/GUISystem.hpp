// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"

#if OPERATING_SYSTEM == OS_WINDOWS
#include "TTauri/GUI/GUISystem_vulkan_win32.hpp"
namespace TTauri {
using GUISystem = GUISystem_vulkan_win32;
}

#elif OPERATING_SYSTEM == OS_MACOS
#include "TTauri/GUI/GUISystem_vulkan_macos.hpp"
namespace TTauri {
using GUISystem = GUISystem_vulkan_macos;
}

#else
#error "GUISystem not implemented for os"
#endif

