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
    text_cursor_left_char,
    text_cursor_right_char,
    text_cursor_down_char,
    text_cursor_up_char,
    text_cursor_left_word,
    text_cursor_right_word,
    text_cursor_begin_line,
    text_cursor_end_line,
    text_cursor_begin_sentence,
    text_cursor_end_sentence,
    text_cursor_begin_document,
    text_cursor_end_document,
    text_select_left_char,
    text_select_right_char,
    text_select_down_char,
    text_select_up_char,
    text_select_word,
    text_select_left_word,
    text_select_right_word,
    text_select_begin_line,
    text_select_end_line,
    text_select_begin_sentence,
    text_select_end_sentence,
    text_select_document,
    text_select_begin_document,
    text_select_end_document,
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
    command::text_cursor_left_char, "text_cursor_left_char",
    command::text_cursor_right_char, "text_cursor_right_char",
    command::text_cursor_down_char, "text_cursor_down_char",
    command::text_cursor_up_char, "text_cursor_up_char",
    command::text_cursor_left_word, "text_cursor_left_word",
    command::text_cursor_right_word, "text_cursor_right_word",
    command::text_cursor_begin_line, "text_cursor_begin_line",
    command::text_cursor_end_line, "text_cursor_end_line",
    command::text_cursor_begin_sentence, "text_cursor_begin_sentence",
    command::text_cursor_end_sentence, "text_cursor_end_sentence",
    command::text_cursor_begin_document, "text_cursor_begin_document",
    command::text_cursor_end_document, "text_cursor_end_document",
    command::text_select_left_char, "text_select_left_char",
    command::text_select_right_char, "text_select_right_char",
    command::text_select_down_char, "text_select_down_char",
    command::text_select_up_char, "text_select_up_char",
    command::text_select_word, "text_select_word",
    command::text_select_left_word, "text_select_left_word",
    command::text_select_right_word, "text_select_right_word",
    command::text_select_begin_line, "text_select_begin_line",
    command::text_select_end_line, "text_select_end_line",
    command::text_select_begin_sentence, "text_select_begin_sentence",
    command::text_select_end_sentence, "text_select_end_sentence",
    command::text_select_begin_document, "text_select_begin_document",
    command::text_select_end_document, "text_select_end_document",
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

[[nodiscard]] constexpr bool is_text_edit_command(command const &rhs) noexcept
{
    using enum tt::command;
    switch (rhs) {
    case text_cursor_left_char:
    case text_cursor_right_char:
    case text_cursor_down_char:
    case text_cursor_up_char:
    case text_cursor_left_word:
    case text_cursor_right_word:
    case text_cursor_begin_line:
    case text_cursor_end_line:
    case text_cursor_begin_sentence:
    case text_cursor_end_sentence:
    case text_cursor_begin_document:
    case text_cursor_end_document:
    case text_select_left_char:
    case text_select_right_char:
    case text_select_down_char:
    case text_select_up_char:
    case text_select_word:
    case text_select_left_word:
    case text_select_right_word:
    case text_select_begin_line:
    case text_select_end_line:
    case text_select_begin_sentence:
    case text_select_end_sentence:
    case text_select_begin_document:
    case text_select_end_document:
    case text_select_document:
    case text_mode_insert:
    case text_delete_char_prev:
    case text_delete_char_next:
    case text_edit_paste:
    case text_edit_copy:
    case text_edit_cut:
    case text_undo:
    case text_redo:
        return true;
    default:
        return false;
    }
}

constexpr command to_command(std::string_view name) noexcept
{
    return command_metadata.at(name, command::unknown);
}

} // namespace tt::inline v1
