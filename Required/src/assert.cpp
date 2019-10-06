// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Required/assert.hpp"
#include "TTauri/Required/globals.hpp"
#include "TTauri/Required/strings.hpp"
#include "TTauri/Required/required.hpp"
#include <fmt/format.h>
#include <ostream>
#include <iostream>

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

static bool backoff(uint64_t count)
{
    return PopulationCount64(count) <= 1;
}

void assert_logging(uint64_t count, char const *source_file, int source_line, char const *expression)
{
    if (Required_globals->assert_logger) {
        if (backoff(count)) {
            Required_globals->assert_logger(source_file, source_line, expression);
        }
    } else {
        if (backoff(count)) {
            let message = fmt::format("Assert failed at {}:{}, count {}: '{}'", source_file, source_line, count, expression);
            std::cerr << message << std::endl;
        }
    }
}

[[noreturn]] void assert_terminating(char const *source_file, int source_line, char const *expression)
{
    assert_logging(0, source_file, source_line, expression);

#if OPERATING_SYSTEM == OS_WINDOWS
    if (IsDebuggerPresent()) {
        let message = fmt::format("Assert failed at {}:{}: '{}'", source_file, source_line, expression);
        let messageWString = translateString<std::wstring>(message);
        OutputDebugStringW(messageWString.data());
        stop_debugger();

    } else {
        let message = fmt::format("Assert failed at {}:{}: '{}'.\n\n"
            "This serious bug in the application, please email support@pokitec.com with the error message above. "
            "Press OK to quit the application.",
            source_file, source_line, expression
        );
        let messageWString = translateString<std::wstring>(message);
        let captionWString = translateString<std::wstring>(std::string("Assert Failed"));
        MessageBoxW(nullptr, messageWString.data(), captionWString.data(), MB_APPLMODAL | MB_OK | MB_ICONERROR);
    }
#elif
#endif
    std::terminate();
}


}
