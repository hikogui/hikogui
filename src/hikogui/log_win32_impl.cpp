// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "win32_headers.hpp"

#include "log.hpp"
#include "trace.hpp"
#include "utility.hpp"
#include "strings.hpp"
#include "thread.hpp"
#include <format>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>

namespace hi::inline v1 {

[[nodiscard]] std::string get_last_error_message() noexcept
{
    DWORD const errorCode = GetLastError();
    std::size_t const messageSize = 32768;
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

    return strip(message);
}

} // namespace hi::inline v1
