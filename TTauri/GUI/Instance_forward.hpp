// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/os_detect.hpp"

namespace TTauri::GUI {

#if OPERATING_SYSTEM == OS_WINDOWS
class Instance_vulkan_win32;
using Instance = Instance_vulkan_win32;

#elif OPERATING_SYSTEM == OS_MACOS
class Instance_vulkan_macos;
using Instance = Instance_vulkan_macos;

#else
#error "Instance forward not implemented for os"
#endif

}
