// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "GUISystem_forward.hpp"
#include "../os_detect.hpp"

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "GUISystem_vulkan_win32.hpp"
namespace tt {
using GUISystem = GUISystem_vulkan_win32;
}

#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
#include "GUISystem_vulkan_macos.hpp"
namespace tt {
using GUISystem = GUISystem_vulkan_macos;
}

#else
#error "GUISystem not implemented for os"
#endif

