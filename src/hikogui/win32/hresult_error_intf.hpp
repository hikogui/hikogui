// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "win32_error_intf.hpp"
#include <Windows.h>
#include <errhandlingapi.h>
#include <system_error>

hi_export_module(hikogui.win32 : win32_error_intf);

hi_export namespace hi { inline namespace v1 {

enum class hresult_error : uint32_t {
    ok = S_OK,
    unspecified_error = E_FAIL,
    invalid_argument = E_INVALIDARG,
};

}} // namespace hi::v1

hi_export template<>
struct std::is_error_code_enum<hi::hresult_error> : std::true_type {};

hi_export namespace hi { inline namespace v1 {

struct hresult_error_category : std::error_category {
    char const *name() const noexcept override
    {
        return "hresult";
    }

    std::string message(int code) const override;

    bool equivalent(int code, std::error_condition const & condition) const noexcept override
    {
        switch (static_cast<hi::hresult_error>(code)) {
        case hi::hresult_error::invalid_argument:
            return condition == std::errc::invalid_argument;
        default:
            return false;
        };
    }
};

hi_inline auto global_hresult_error_category = hresult_error_category{};

[[nodiscard]] hi_inline std::error_code make_error_code(hresult_error code) noexcept
{
    return {static_cast<int>(code), global_hresult_error_category};
}

[[nodiscard]] hi_inline hresult_error to_win32_error(win32_error code) noexcept
{
    return static_cast<hresult_error>(__HRESULT_FROM_WIN32(static_cast<DWORD>(code)));
}

}}
