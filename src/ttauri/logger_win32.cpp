// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "logger.hpp"
#include "trace.hpp"
#include "cpu_utc_clock.hpp"
#include "required.hpp"
#include "URL.hpp"
#include "strings.hpp"
#include "thread.hpp"
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

std::string getLastErrorMessage()
{
    DWORD const errorCode = GetLastError();
    size_t const messageSize = 32768;
    wchar_t *const c16_message = new wchar_t[messageSize];

    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // source
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        c16_message,
        messageSize,
        NULL);

    auto message = to_string(std::wstring(c16_message));
    delete[] c16_message;

    return message;
}

void logger_init_system() {
    // if (!debugger_is_present() && AttachConsole(ATTACH_PARENT_PROCESS)) {
//    FILE *fpstdin = stdin, *fpstdout = stdout, *fpstderr = stderr;
//
//    freopen_s(&fpstdin, "CONIN$", "r", stdin);
//    freopen_s(&fpstdout, "CONOUT$", "w", stdout);
//    freopen_s(&fpstderr, "CONOUT$", "w", stderr);
//}
}

} // namespace tt
