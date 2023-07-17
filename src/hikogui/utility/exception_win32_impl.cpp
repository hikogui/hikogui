// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

import hikogui_utility_defer;
#include "exception.hpp"
#include "win32_headers.hpp"

namespace hi {
inline namespace v1 {

[[nodiscard]] std::string get_last_error_message(uint32_t error_code)
{
    hilet error_code_ = narrow_cast<DWORD>(error_code);

    // FormatMessageW() is unable to tell what the buffer size should be.
    // But 64Kbyte is the largest buffer that one should pass.
    LPWSTR buffer = nullptr;
    hilet result = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // source
        error_code_,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer),
        0,
        NULL);

    if (result == 0) {
        throw os_error(std::format("Could not format OS error message for error {}.", error_code));
    }

    hilet d = defer([&]{
        LocalFree(buffer);
    });

    return win32_wstring_to_string(std::wstring_view{buffer, result});
}

[[nodiscard]] uint32_t get_last_error_code() noexcept
{
    return narrow_cast<uint32_t>(GetLastError());
}

[[nodiscard]] std::string get_last_error_message() {
    return get_last_error_message(get_last_error_code());
}

}}
