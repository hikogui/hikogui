#pragma once

#include "locked_memory_allocator_intf.hpp" // export
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "locked_memory_allocator_win32_impl.hpp" // export
#endif

hi_export_module(hikogui.memory.locked_memory_allocator);
