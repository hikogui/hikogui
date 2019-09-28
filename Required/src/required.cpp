// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Required/required.hpp"

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#include <debugapi.h>
#endif

namespace TTauri {

void stop_debugger()
{
#if OPERATING_SYSTEM == OS_WINDOWS
    DebugBreak();
#endif
}


}
