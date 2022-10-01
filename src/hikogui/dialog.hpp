// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string_view>
#include <iostream>
#include <format>

namespace hi::inline v1 {

enum class dialog_type { ok, yes_no, yes_no_cancel };

/** Display a modal dialog.
 * You should never display a modal dialog unless it is absolutely
 * necessary for the user to respond right now, or on a catastrophic failure.
 *
 * @throw cancel_error When the user presses "cancel".
 * @return True when the user presses "ok" or "yes"
 */
bool dialog(dialog_type type, std::string_view title, std::string_view text);

inline void dialog_ok(std::string_view title, std::string_view message) noexcept
{
    dialog(dialog_type::ok, title, message);
}

[[nodiscard]] inline bool dialog_yes_no(std::string_view title, std::string_view message) noexcept
{
    return dialog(dialog_type::yes_no, title, message);
}

[[nodiscard]] inline bool dialog_yes_no_cancel(std::string_view title, std::string_view message)
{
    return dialog(dialog_type::yes_no_cancel, title, message);
}

} // namespace hi::inline v1
