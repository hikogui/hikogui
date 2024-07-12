#pragma once

#include "../path/path_location.hpp" // XXX #616
#include "user_settings_intf.hpp" // export
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "user_settings_win32_impl.hpp" // export
#endif

hi_export_module(hikogui.settings.user_settings);
