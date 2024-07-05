

#pragma once

#include "path_location_intf.hpp" // export
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "path_location_win32_impl.hpp" // export
#endif

hi_export_module(hikogui.path.path_location);
