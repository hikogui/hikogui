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

void _debugger_log(char const *message) noexcept
{
    ttlet messageString = std::string(message) + "\r\n";
    ttlet messageWString = to_wstring(messageString);
    OutputDebugStringW(messageWString.data());
}

void _debugger_dialogue(char const *caption, char const *message)
{
    ttlet captionString = std::string(caption);
    ttlet messageString = std::string(message);
    ttlet captionWString = to_wstring(captionString);
    ttlet messageWString = to_wstring(messageString);
    MessageBoxW(nullptr, messageWString.data(), captionWString.data(), MB_APPLMODAL | MB_OK | MB_ICONERROR);
}

void _debugger_break() 
{
    DebugBreak();
}

}
