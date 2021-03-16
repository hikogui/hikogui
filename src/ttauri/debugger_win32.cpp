// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "debugger.hpp"
#include "strings.hpp"
#include <Windows.h>
#include <debugapi.h>

namespace tt {

bool debugger_is_present() noexcept {
    return IsDebuggerPresent();
}

void _debugger_break() 
{
    DebugBreak();
}

}
