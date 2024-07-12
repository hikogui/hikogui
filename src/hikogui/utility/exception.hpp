
#pragma once

#include "exception_intf.hpp" // export
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "exception_win32_impl.hpp" // export
#endif

hi_export_module(hikogui.utility.exception);
