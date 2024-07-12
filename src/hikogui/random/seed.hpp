#pragma once

#include "seed_intf.hpp" // export
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "seed_win32_impl.hpp" // export
#endif

hi_export_module(hikogui.random.seed);
