

#pragma once

hi_export_module(hikogui.dispatch.socket_event);

#include "socket_event_intf.hpp" // export
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "socket_event_win32_impl.hpp" // export
#endif
