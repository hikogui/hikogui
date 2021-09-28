// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string_view>
#include <iostream>
#include <format>

namespace tt {

enum class dialog_type {
    ok,
    yes_no,
    yes_no_cancel
};

/** Display a modal dialog.
 * You should never display a modal dialog unless it is absolutely
 * necessary for the user to respond right now, or on a catastrophic failure.
 * 
 * @throw cancel_error When the user presses "cancel".
 * @return True when the user presses "ok" or "yes"
 */
bool _dialog(dialog_type type, char const *title, std::string_view text);

template<typename... Args>
void dialog_ok(char const *title, std::string_view fmt, Args const &...args) noexcept
{
    _dialog(dialog_type::ok, title, std::format(fmt, args...));
}

template<typename... Args>
[[nodiscard]] bool dialog_yes_no(char const *title, char const *fmt, Args const &...args) noexcept
{
    return _dialog(dialog_type::yes_no, title, std::format(fmt, args...));
}

template<typename... Args>
[[nodiscard]] bool dialog_yes_no_cancel(char const *title, char const *fmt, Args const &...args)
{
    return _dialog(dialog_type::yes_no_cancel, title, std::format(fmt, args...));
}

} // namespace tt
