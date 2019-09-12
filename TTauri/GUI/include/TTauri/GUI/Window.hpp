// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/os_detect.hpp"

#if OPERATING_SYSTEM == OS_WINDOWS
#include "Window_vulkan_win32.hpp"
namespace TTauri::GUI {
using Window = Window_vulkan_win32;
}

#elif OPERATING_SYSTEM == OS_MACOS
#include "Window_vulkan_macos.hpp"
namespace TTauri::GUI {
using Window = Window_vulkan_macos;
}

#else
#error "Window not implemented for this os."
#endif
