// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/logger.hpp"
#include "ttauri/foundation/trace.hpp"
#include "ttauri/foundation/cpu_utc_clock.hpp"
#include "ttauri/foundation/globals.hpp"
#include "ttauri/foundation/required.hpp"
#include "ttauri/foundation/URL.hpp"
#include "ttauri/foundation/strings.hpp"
#include "ttauri/foundation/thread.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>
#include <Windows.h>
#include <debugapi.h>

namespace tt {

using namespace std::literals::chrono_literals;


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

    ttlet message = to_string(std::wstring(c16_message));
    delete [] c16_message;

    return message;
}


}
