// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../os_detect.hpp"

namespace tt {

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
class GUISystem_vulkan_win32;
using GUISystem = GUISystem_vulkan_win32;

#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
class GUISystem_vulkan_macos;
using GUISystem = GUISystem_vulkan_macos;

#else
#error "GUISystem forward not implemented for os"
#endif

}

namespace tt {

/*! Global mutex for GUI functionality.
*/
inline std::recursive_mutex guiMutex;

}
