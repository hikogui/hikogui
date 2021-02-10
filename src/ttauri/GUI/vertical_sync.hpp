// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../os_detect.hpp"

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "vertical_sync_win32.hpp"
namespace tt {
using vertical_sync = vertical_sync_win32;
}

#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
#include "vertical_sync_macos.hpp"
namespace tt {
using vertical_sync = vertical_sync_macos;
}

#else
#error "vertical_sync not implemented for os"
#endif
