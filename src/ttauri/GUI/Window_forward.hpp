// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../os_detect.hpp"

namespace tt {

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
class Window_vulkan_win32;
using Window = Window_vulkan_win32;

#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
class Window_vulkan_macos;
using Window = Window_vulkan_macos;

#else
#error "Window forward not implemeted."
#endif

}
