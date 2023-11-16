// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "base.hpp"
#include <expected>
#include <string>
#include <system_error>

hi_export_module(hikogui.win32.synchapi);

hi_export namespace hi {
inline namespace v1 {

[[nodiscard]] hi_inline std::expected<HANDLE, win32_error> win32_CreateEvent(
    SECURITY_ATTRIBUTES const *event_attributes = nullptr,
    bool manual_reset = true,
    bool initial_state = false,
    std::optional<std::string> name = std::nullopt) noexcept
{
    auto name_wstr = std::wstring{};
    wchar_t const *name_cstr = nullptr;
    if (name) {
        if (auto name_wstr_ = win32_MultiByteToWideChar(*name)) {
            name_wstr = *name_wstr_;
            name_cstr = name_wstr.c_str();
        } else {
            return std::unexpected{name_wstr_.error()};
        }
    }

    if (auto r = CreateEventW(const_cast<SECURITY_ATTRIBUTES *>(event_attributes), manual_reset, initial_state, name_cstr); r != NULL) {
        return r;
    } else {
        return std::unexpected{win32_GetLastError()};
    }
}

} // namespace v1
}
