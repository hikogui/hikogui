

#pragma once

#include "os_settings_intf.hpp" // export
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "os_settings_win32_impl.hpp" // export
#endif

hi_export_module(hikogui.settings.os_settings);
