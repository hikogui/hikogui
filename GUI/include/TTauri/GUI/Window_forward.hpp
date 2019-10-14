// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"

namespace TTauri::GUI {

#if OPERATING_SYSTEM == OS_WINDOWS
class Window_vulkan_win32;
using Window = Window_vulkan_win32;

#elif OPERATING_SYSTEM == OS_MACOS
class Window_vulkan_macos;
using Window = Window_vulkan_macos;

#else
#error "Window forward not implemeted."
#endif

}
