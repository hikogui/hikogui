// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../os_detect.hpp"

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "VerticalSync_win32.hpp"
namespace tt {
using VerticalSync = VerticalSync_win32;
}

#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
#include "VerticalSync_macos.hpp"
namespace tt {
using VerticalSync = VerticalSync_macos;
}

#else
#error "VerticalSync not implemented for os"
#endif
