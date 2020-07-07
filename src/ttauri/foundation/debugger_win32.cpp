// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/debugger.hpp"
#include "ttauri/foundation/strings.hpp"
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
