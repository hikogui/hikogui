// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "TTauri/GUI/GUISystem_vulkan_win32.hpp"
namespace tt {
using GUISystem = GUISystem_vulkan_win32;
}

#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
#include "TTauri/GUI/GUISystem_vulkan_macos.hpp"
namespace tt {
using GUISystem = GUISystem_vulkan_macos;
}

#else
#error "GUISystem not implemented for os"
#endif

