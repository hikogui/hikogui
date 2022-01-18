// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "dialog.hpp"
#include "exception.hpp"
#include "unicode/UTF.hpp"
#include <Windows.h>
#include <WinUser.h>

namespace tt::inline v1 {

bool _dialog(dialog_type type, const char *title, std::string_view text)
{
    UINT type_;

    switch (type) {
    case dialog_type::ok:
        // Just "ok" can only be a notification, so it must be an error as well.
        type_ = MB_APPLMODAL | MB_OK | MB_ICONERROR;
        break;
    case dialog_type::yes_no:
        // Just "yes" / "no" is serious, so exclamation.
        type_ = MB_APPLMODAL | MB_YESNO | MB_ICONEXCLAMATION;
        break;

    case dialog_type::yes_no_cancel:
        // If we can cancel it must be a warning.
        type_ = MB_APPLMODAL | MB_YESNOCANCEL | MB_ICONWARNING;
        break;

    default: tt_no_default();
    }

    ttlet title_ = tt::to_wstring(title);
    ttlet text_ = tt::to_wstring(text);
    ttlet r = MessageBoxW(nullptr, text_.c_str(), title_.c_str(), type_);

    switch (r) {
    case IDABORT:
    case IDCANCEL: throw cancel_error("User pressed cancel");
    case IDCONTINUE:
    case IDOK:
    case IDYES: return true;
    case IDNO: return false;
    default: tt_no_default();
    }
}

} // namespace tt::inline v1
