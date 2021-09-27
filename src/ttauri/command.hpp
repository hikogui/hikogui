// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include <string>
#include <string_view>
#include <ostream>

namespace tt {

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
    gui_toolbar_next,
    gui_toolbar_prev,
    gui_activate,
    gui_enter,
    gui_escape,
};

constexpr char const *to_const_string(command rhs) noexcept
{
    switch (rhs) {
    case command::unknown: return "unknown";
    case command::text_cursor_char_left: return "text_cursor_char_left";
    case command::text_cursor_char_right: return "text_cursor_char_right";
    case command::text_cursor_word_left: return "text_cursor_word_left";
    case command::text_cursor_word_right: return "text_cursor_word_right";
    case command::text_cursor_line_begin: return "text_cursor_line_begin";
    case command::text_cursor_line_end: return "text_cursor_line_end";
    case command::text_select_char_left: return "text_select_char_left";
    case command::text_select_char_right: return "text_select_char_right";
    case command::text_select_word: return "text_select_word";
    case command::text_select_word_left: return "text_select_word_left";
    case command::text_select_word_right: return "text_select_word_right";
    case command::text_select_line_begin: return "text_select_line_begin";
    case command::text_select_line_end: return "text_select_line_end";
    case command::text_select_document: return "text_select_document";
    case command::text_mode_insert: return "text_mode_insert";
    case command::text_delete_char_prev: return "text_delete_char_prev";
    case command::text_delete_char_next: return "text_delete_char_next";
    case command::text_edit_paste: return "text_edit_paste";
    case command::text_edit_copy: return "text_edit_copy";
    case command::text_edit_cut: return "text_edit_cut";
    case command::text_undo: return "text_undo";
    case command::text_redo: return "text_redo";
    case command::gui_keyboard_enter: return "gui_keyboard_enter";
    case command::gui_keyboard_exit: return "gui_keyboard_exit";
    case command::gui_mouse_enter: return "gui_mouse_enter";
    case command::gui_mouse_exit: return "gui_mouse_exit";
    case command::gui_widget_next: return "gui_widget_next";
    case command::gui_widget_prev: return "gui_widget_prev";
    case command::gui_menu_next: return "gui_menu_next";
    case command::gui_menu_prev: return "gui_menu_prev";
    case command::gui_toolbar_next: return "gui_toolbar_next";
    case command::gui_toolbar_prev: return "gui_toolbar_prev";
    case command::gui_activate: return "gui_activate";
    case command::gui_enter: return "gui_enter";
    case command::gui_escape: return "gui_escape";
    default:
        tt_no_default();
    }
}

inline std::string to_string(command rhs) noexcept {
    return to_const_string(rhs);
}

inline std::ostream &operator<<(std::ostream &lhs, command const &rhs) {
    return lhs << to_const_string(rhs);
}

constexpr command to_command(std::string_view name) noexcept
{
    if (name == "text_cursor_char_left") {
        return command::text_cursor_char_left;
    } else if (name == "text_cursor_char_right") {
        return command::text_cursor_char_right;
    } else if (name == "text_cursor_word_left") {
        return command::text_cursor_word_left;
    } else if (name == "text_cursor_word_right") {
        return command::text_cursor_word_right;
    } else if (name == "text_cursor_line_begin") {
        return command::text_cursor_line_begin;
    } else if (name == "text_cursor_line_end") {
        return command::text_cursor_line_end;
    } else if (name == "text_select_char_left") {
        return command::text_select_char_left;
    } else if (name == "text_select_char_right") {
        return command::text_select_char_right;
    } else if (name == "text_select_word") {
        return command::text_select_word;
    } else if (name == "text_select_word_left") {
        return command::text_select_word_left;
    } else if (name == "text_select_word_right") {
        return command::text_select_word_right;
    } else if (name == "text_select_line_begin") {
        return command::text_select_line_begin;
    } else if (name == "text_select_line_end") {
        return command::text_select_line_end;
    } else if (name == "text_select_document") {
        return command::text_select_document;
    } else if (name == "text_mode_insert") {
        return command::text_mode_insert;
    } else if (name == "text_delete_char_prev") {
        return command::text_delete_char_prev;
    } else if (name == "text_delete_char_next") {
        return command::text_delete_char_next;
    } else if (name == "text_edit_paste") {
        return command::text_edit_paste;
    } else if (name == "text_edit_copy") {
        return command::text_edit_copy;
    } else if (name == "text_edit_cut") {
        return command::text_edit_cut;
    } else if (name == "text_undo") {
        return command::text_undo;
    } else if (name == "text_redo") {
        return command::text_redo;
    } else if (name == "gui_keyboard_enter") {
        return command::gui_keyboard_enter;
    } else if (name == "gui_keyboard_exit") {
        return command::gui_keyboard_exit;
    } else if (name == "gui_mouse_enter") {
        return command::gui_mouse_enter;
    } else if (name == "gui_mouse_exit") {
        return command::gui_mouse_exit;
    } else if (name == "gui_widget_next") {
        return command::gui_widget_next;
    } else if (name == "gui_widget_prev") {
        return command::gui_widget_prev;
    } else if (name == "gui_menu_next") {
        return command::gui_menu_next;
    } else if (name == "gui_menu_prev") {
        return command::gui_menu_prev;
    } else if (name == "gui_toolbar_next") {
        return command::gui_toolbar_next;
    } else if (name == "gui_toolbar_prev") {
        return command::gui_toolbar_prev;
    } else if (name == "gui_activate") {
        return command::gui_activate;
    } else if (name == "gui_enter") {
        return command::gui_enter;
    } else if (name == "gui_escape") {
        return command::gui_escape;
    } else {
        return command::unknown;
    }
}

}
