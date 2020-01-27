// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/debugger.hpp"
#include "TTauri/Foundation/strings.hpp"
#include <Windows.h>
#include <debugapi.h>

namespace TTauri {

bool debugger_is_present() noexcept {
    return IsDebuggerPresent();
}

void _debugger_log(char const *message) noexcept
{
    let messageString = std::string(message) + "\r\n";
    let messageWString = to_wstring(messageString);
    OutputDebugStringW(messageWString.data());
}

void _debugger_dialogue(char const *caption, char const *message)
{
    let captionString = std::string(caption);
    let messageString = std::string(message);
    let captionWString = to_wstring(captionString);
    let messageWString = to_wstring(messageString);
    MessageBoxW(nullptr, messageWString.data(), captionWString.data(), MB_APPLMODAL | MB_OK | MB_ICONERROR);
}

void _debugger_break() 
{
    DebugBreak();
}

}
