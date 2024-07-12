
#pragma once

#include "crt_utils_intf.hpp" // export
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "crt_utils_win32_impl.hpp" // export
#endif

hi_export_module(hikogui.crt.crt_utils);
