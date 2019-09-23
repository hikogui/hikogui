// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Diagnostic/logger.hpp"
#include "TTauri/Diagnostic/trace.hpp"
#include "TTauri/Time/cpu_utc_clock.hpp"
#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/required.hpp"
#include "TTauri/Required/URL.hpp"
#include "TTauri/Required/strings.hpp"
#include "TTauri/Required/thread.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>
#include <Windows.h>
#include <debugapi.h>

namespace TTauri {

using namespace std::literals::chrono_literals;

void logger_type::writeToConsole(std::string str) noexcept {
    str += "\r\n";
    OutputDebugStringW(translateString<std::wstring>(str).data());
}

gsl_suppress(i.11)
std::string getLastErrorMessage()
{
    DWORD const errorCode = GetLastError();
    size_t const messageSize = 32768;
    wchar_t* const c16_message = new wchar_t[messageSize];;

    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // source
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        c16_message,
        messageSize,
        NULL
    );

    let message = translateString<std::string>(std::wstring(c16_message));
    delete [] c16_message;

    return message;
}

}
