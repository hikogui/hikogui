// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include <Windows.h>
#include <errhandlingapi.h>
#include <system_error>

hi_export_module(hikogui.win32 : win32_error_intf);

hi_export namespace hi { inline namespace v1 {

enum class win32_error : uint32_t {
    success = ERROR_SUCCESS,
    file_not_found = ERROR_FILE_NOT_FOUND,
    more_data = ERROR_MORE_DATA,
    invalid_data = ERROR_INVALID_DATA,
    insufficient_buffer = ERROR_INSUFFICIENT_BUFFER,
    status_pending = STATUS_PENDING,
};

}} // namespace hi::v1

hi_export template<>
struct std::is_error_code_enum<hi::win32_error> : std::true_type {};

hi_export namespace hi { inline namespace v1 {

struct win32_error_category : std::error_category {
    char const *name() const noexcept override
    {
        return "win32";
    }

    std::string message(int code) const override;

    bool equivalent(int code, std::error_condition const & condition) const noexcept override
    {
        switch (static_cast<hi::win32_error>(code)) {
        case hi::win32_error::file_not_found:
            return condition == std::errc::no_such_file_or_directory;
        case hi::win32_error::more_data:
            return condition == std::errc::message_size;
        case hi::win32_error::invalid_data:
            return condition == std::errc::bad_message;
        case hi::win32_error::status_pending:
            return condition == std::errc::interrupted;
        case hi::win32_error::insufficient_buffer:
            return condition == std::errc::no_buffer_space;
        default:
            return false;
        };
    }
};

inline auto global_win32_error_category = win32_error_category{};

[[nodiscard]] inline std::error_code make_error_code(win32_error code) noexcept
{
    return {static_cast<int>(code), global_win32_error_category};
}

[[nodiscard]] inline win32_error win32_GetLastError() noexcept
{
    return static_cast<win32_error>(::GetLastError());
}

}}
