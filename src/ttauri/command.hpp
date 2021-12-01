// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "enum_metadata.hpp"
#include <string>
#include <string_view>
#include <ostream>

namespace tt::inline v1 {

enum class command {
    unknown,
    text_cursor_char_left,
    text_cursor_char_right,
    text_cursor_word_left,
    text_cursor_word_right,
    text_cursor_line_begin,
    text_cursor_line_end,
    text_select_char_left,
    text_select_char_right,
    text_select_word,
    text_select_word_left,
    text_select_word_right,
    text_select_line_begin,
    text_select_line_end,
    text_select_document,
    text_mode_insert,
    text_delete_char_prev,
    text_delete_char_next,
    text_edit_paste,
    text_edit_copy,
    text_edit_cut,
    text_undo,
    text_redo,
    gui_keyboard_enter,
    gui_keyboard_exit,
    gui_mouse_enter,
    gui_mouse_exit,
    gui_widget_next,
    gui_widget_prev,
    gui_menu_next,
    gui_menu_prev,
    gui_sysmenu_open,
    gui_toolbar_open,
    gui_toolbar_next,
    gui_toolbar_prev,
    gui_activate,
    gui_enter,
    gui_cancel,
};

// clang-format off
constexpr auto command_metadata = enum_metadata{
    command::unknown, "unknown",
    command::text_cursor_char_left, "text_cursor_char_left",
    command::text_cursor_char_right, "text_cursor_char_right",
    command::text_cursor_word_left, "text_cursor_word_left",
    command::text_cursor_word_right, "text_cursor_word_right",
    command::text_cursor_line_begin, "text_cursor_line_begin",
    command::text_cursor_line_end, "text_cursor_line_end",
    command::text_select_char_left, "text_select_char_left",
    command::text_select_char_right, "text_select_char_right",
    command::text_select_word, "text_select_word",
    command::text_select_word_left, "text_select_word_left",
    command::text_select_word_right, "text_select_word_right",
    command::text_select_line_begin, "text_select_line_begin",
    command::text_select_line_end, "text_select_line_end",
    command::text_select_document, "text_select_document",
    command::text_mode_insert, "text_mode_insert",
    command::text_delete_char_prev, "text_delete_char_prev",
    command::text_delete_char_next, "text_delete_char_next",
    command::text_edit_paste, "text_edit_paste",
    command::text_edit_copy, "text_edit_copy",
    command::text_edit_cut, "text_edit_cut",
    command::text_undo, "text_undo",
    command::text_redo, "text_redo",
    command::gui_keyboard_enter, "gui_keyboard_enter",
    command::gui_keyboard_exit, "gui_keyboard_exit",
    command::gui_mouse_enter, "gui_mouse_enter",
    command::gui_mouse_exit, "gui_mouse_exit",
    command::gui_widget_next, "gui_widget_next",
    command::gui_widget_prev, "gui_widget_prev",
    command::gui_menu_next, "gui_menu_next",
    command::gui_menu_prev, "gui_menu_prev",
    command::gui_sysmenu_open, "gui_sysmenu_open",
    command::gui_toolbar_open, "gui_toolbar_open",
    command::gui_toolbar_next, "gui_toolbar_next",
    command::gui_toolbar_prev, "gui_toolbar_prev",
    command::gui_activate, "gui_activate",
    command::gui_enter, "gui_enter",
    command::gui_cancel, "gui_cancel"
};
// clang-format on

inline std::string_view to_string(command rhs) noexcept
{
    return command_metadata[rhs];
}

inline std::ostream &operator<<(std::ostream &lhs, command const &rhs)
{
    return lhs << command_metadata[rhs];
}

constexpr command to_command(std::string_view name) noexcept
{
    return command_metadata.at(name, command::unknown);
}

} // namespace tt::inline v1
