
#pragma once

#include "dialog_intf.hpp" // export

#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "dialog_win32_impl.hpp" // export
#endif

hi_export_module(hikogui.utility.dialog);
