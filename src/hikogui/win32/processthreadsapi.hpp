// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "base.hpp"
#include <expected>
#include <string>
#include <system_error>

hi_export_module(hikogui.win32.processthreadsapi);

hi_export namespace hi {
inline namespace v1 {

[[nodiscard]] hi_inline std::expected<uint32_t, win32_error> win32_GetExitCodeProcess(HANDLE process_handle) noexcept
{
    DWORD exit_code = 0;
    if (GetExitCodeProcess(process_handle, &exit_code)) {
        if (exit_code == STILL_ACTIVE) {
            return std::unexpected{win32_error::status_pending};
        } else {
            return exit_code;
        }
    } else {
        return std::unexpected{win32_GetLastError()};
    }
}

template<typename StartupInfo>
[[nodiscard]] hi_inline std::expected<PROCESS_INFORMATION, win32_error> win32_CreateProcess(
    std::optional<std::string> application_name,
    std::optional<std::string> command_line,
    SECURITY_ATTRIBUTES const *process_attributes,
    SECURITY_ATTRIBUTES const *thread_attributes,
    bool inherit_handles,
    uint32_t creation_flags,
    void const *environment,
    std::optional<std::string> current_directory,
    StartupInfo const &startup_info)
{
    auto application_name_wstr = std::wstring{};
    wchar_t const *application_name_cstr = nullptr;
    if (application_name) {
        if (auto application_name_wstr_ = win32_MultiByteToWideChar(*application_name)) {
            application_name_wstr = *application_name_wstr_;
            application_name_cstr = application_name_wstr.c_str();
        } else {
            return std::unexpected{application_name_wstr_.error()};
        }
    }

    auto command_line_wstr = std::wstring{};
    wchar_t *command_line_cstr = nullptr;
    if (command_line) {
        if (auto command_line_wstr_ = win32_MultiByteToWideChar(*command_line)) {
            command_line_wstr = *command_line_wstr_;
            command_line_cstr = command_line_wstr.data();
        } else {
            return std::unexpected{command_line_wstr_.error()};
        }
    }

    auto current_directory_wstr = std::wstring{};
    wchar_t *current_directory_cstr = nullptr;
    if (current_directory) {
        if (auto current_directory_wstr_ = win32_MultiByteToWideChar(*current_directory)) {
            current_directory_wstr = *current_directory_wstr_;
            current_directory_cstr = current_directory_wstr.data();
        } else {
            return std::unexpected{current_directory_wstr_.error()};
        }
    }

    auto r = PROCESS_INFORMATION{};

    if (not CreateProcessW(
            application_name_cstr,
            command_line_cstr,
            const_cast<SECURITY_ATTRIBUTES *>(process_attributes),
            const_cast<SECURITY_ATTRIBUTES *>(thread_attributes),
            inherit_handles,
            static_cast<DWORD>(creation_flags),
            const_cast<void *>(environment),
            current_directory_cstr,
            const_cast<STARTUPINFOW *>(reinterpret_cast<STARTUPINFOW const *>(&startup_info)),
            &r)) {
        return std::unexpected{win32_GetLastError()};
    }

    return r;
}

} // namespace v1
}
