// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"

namespace TTauri {

#if OPERATING_SYSTEM == OS_WINDOWS
class GUISystem_vulkan_win32;
using GUISystem = GUISystem_vulkan_win32;

#elif OPERATING_SYSTEM == OS_MACOS
class GUISystem_vulkan_macos;
using GUISystem = GUISystem_vulkan_macos;

#else
#error "GUISystem forward not implemented for os"
#endif

}
