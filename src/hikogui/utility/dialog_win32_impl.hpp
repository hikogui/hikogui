// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"

#include "dialog_intf.hpp"
#include "../win32/win32.hpp"
#include "../macros.hpp"
#include <string_view>
#include <expected>
#include <system_error>

hi_export_module(hikogui.utility.dialog : impl);

hi_export namespace hi { inline namespace v1 {

hi_export hi_inline std::expected<dialog_button, std::error_code> dialog(std::string_view title, std::string_view text, dialog_button_mask button_mask) noexcept
{
    UINT type = 0;

    switch (button_mask) {
    case dialog_button_mask::cancel_retry_continue:
        // Just "ok" can only be a notification, so it must be an error as well.
        type = MB_APPLMODAL | MB_CANCELTRYCONTINUE | MB_ICONWARNING;
        break;

    case dialog_button_mask::ok:
        // Just "ok" can only be a notification, so it must be an error as well.
        type = MB_APPLMODAL | MB_OK | MB_ICONINFORMATION;
        break;

    case dialog_button_mask::ok_cancel:
        // Just "ok" / "cancel" is a serious request to the user.
        type = MB_APPLMODAL | MB_OKCANCEL | MB_ICONEXCLAMATION;
        break;

    case dialog_button_mask::retry_cancel:
        // Just "retry" / "cancel" there was an error.
        type = MB_APPLMODAL | MB_OKCANCEL | MB_ICONWARNING;
        break;

    case dialog_button_mask::yes_no:
        // Just "yes" / "no" is serious, so exclamation.
        type = MB_APPLMODAL | MB_YESNO | MB_ICONQUESTION;
        break;

    case dialog_button_mask::yes_no_cancel:
        // If we can cancel it must be a warning.
        type = MB_APPLMODAL | MB_YESNOCANCEL | MB_ICONQUESTION;
        break;

    default:
        std::terminate();
    }

    if (hilet r = win32_MessageBox(nullptr, text, title, type)) {
        switch (*r) {
        case IDABORT: return dialog_button::cancel;
        case IDCANCEL: return dialog_button::cancel;
        case IDCONTINUE: return dialog_button::_continue;
        case IDIGNORE: return dialog_button::_continue;
        case IDNO: return dialog_button::no;
        case IDOK: return dialog_button::ok;
        case IDRETRY: return dialog_button::retry;
        case IDTRYAGAIN: return dialog_button::retry;
        case IDYES: return dialog_button::yes;
        default: std::terminate();
        }
    } else {
        return std::unexpected{make_error_code(r.error())};
    }
}

}} // namespace hi::inline v1
