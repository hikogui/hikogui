// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "hresult_error_intf.hpp"
#include "winbase.hpp"
#include <format>
#include <string>

hi_export_module(hikogui.win32 : win32_error_impl);

hi_export namespace hi {
inline namespace v1 {

[[nodiscard]] hi_inline std::string hresult_error_category::message(int code) const
{
    auto const code_ = static_cast<uint32_t>(code);

    auto const value = code & 0xffff;
    auto const facility = (code >> 16) & 0x7ff;
    auto const is_message_id = ((code >> 27) & 1) != 0;
    auto const is_ntstatus = ((code >> 28) & 1) != 0;
    auto const is_custom = ((code >> 29) & 1) != 0;
    auto const is_severe_failure = ((code >> 30) & 1) != 0;
    auto const is_failure = ((code >> 31) & 1) != 0;

    if (not is_ntstatus and not is_custom and not is_message_id and facility == 7) {
        if (auto msg = win32_FormatMessage(static_cast<win32_error>(value))) {
            return *msg;

        } else {
            throw std::system_error(msg.error());
        }
    }

    auto const facility_str = [&] {
        if (is_ntstatus) {
            return std::string{"NTSTATUS"};
        } else if (is_custom) {
            return std::string{"Custom"};
        } else if (is_message_id) {
            return std::string{"Message ID"};
        } else {
            switch (facility) {
            case 1:
                return std::string{"RPC"};
            case 2:
                return std::string{"COM Dispatch"};
            case 3:
                return std::string{"OLE Storage"};
            case 4:
                return std::string{"COM/OLE Interface Management"};
            case 7:
                return std::string{"Win32"};
            case 8:
                return std::string{"Windows"};
            case 9:
                return std::string{"SSPI"};
            case 10:
                return std::string{"Control"};
            case 11:
                return std::string{"Client or Server Certificate"};
            default:
                return std::format("Unknown Facility {}", facility);
            }
        }
    }();

    auto const failure_str = [&] {
        if (is_failure) {
            if (is_severe_failure) {
                return "FATAL";
            } else {
                return "ERROR";
            }
        } else {
            return "SUCCESS";
        }
    }();

    return std::format("HRESULT({}): {}: 0x{:08x}", facility_str, failure_str, code_);
}

} // namespace v1
}
