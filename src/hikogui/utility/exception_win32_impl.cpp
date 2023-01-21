// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "exception.hpp"
#include "win32_headers.hpp"

namespace hi {
inline namespace v1 {

[[nodiscard]] std::string get_last_error_message() noexcept
{
    DWORD const errorCode = GetLastError();

    auto message = std::wstring{32768, L'\0'};

    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // source
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        message.data(),
        narrow_cast<DWORD>(message.size()),
        NULL);

    return win32_wstring_to_string(message);
}

}}
