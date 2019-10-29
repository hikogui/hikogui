// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/debugger.hpp"
#include "TTauri/Foundation/strings.hpp"
#include <Windows.h>
#include <debugapi.h>

namespace TTauri {

void _debugger_log(char const *text) noexcept
{
    str += "\r\n";
    OutputDebugStringW(translateString<std::wstring>(text).data());
}

#if !defined(NDEBUG)
void _debugger_dialogue(char const *caption, char const *message)
{
    let captionWString = translateString<std::wstring>(title);
    let messageWString = translateString<std::wstring>(message);
    MessageBoxW(nullptr, messageWString.data(), captionWString.data(), MB_APPLMODAL | MB_OK | MB_ICONERROR);
}
#endif

void _debugger_break() 
{
#if !defined(NDEBUG)
    DebugBreak();
#else
    std::terminate();
#endif
}

}
