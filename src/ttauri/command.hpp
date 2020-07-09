// Copyright 2020 Pokitec
// All rights reserved.

#include "required.hpp"
#include <string>
#include <string_view>
#include <ostream>

#pragma once

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
    gui_widget_next,
    gui_widget_prev,
    gui_activate,
};

constexpr char const *to_const_string(command rhs) noexcept
{
    switch (rhs) {
    case command::unknown: return "unknown";
    case command::text_cursor_char_left: return "text.cursor.char.left";
    case command::text_cursor_char_right: return "text.cursor.char.right";
    case command::text_cursor_word_left: return "text.cursor.word.left";
    case command::text_cursor_word_right: return "text.cursor.word.right";
    case command::text_cursor_line_begin: return "text.cursor.line.begin";
    case command::text_cursor_line_end: return "text.cursor.line.end";
    case command::text_select_char_left: return "text.select.char.left";
    case command::text_select_char_right: return "text.select.char.right";
    case command::text_select_word: return "text.select.word";
    case command::text_select_word_left: return "text.select.word.left";
    case command::text_select_word_right: return "text.select.word.right";
    case command::text_select_line_begin: return "text.select.line.begin";
    case command::text_select_line_end: return "text.select.line.end";
    case command::text_select_document: return "text.select.document";
    case command::text_mode_insert: return "text.mode.insert";
    case command::text_delete_char_prev: return "text.delete.char.prev";
    case command::text_delete_char_next: return "text.delete.char.next";
    case command::text_edit_paste: return "text.edit.paste";
    case command::text_edit_copy: return "text.edit.copy";
    case command::text_edit_cut: return "text.edit.cut";
    case command::text_undo: return "text.undo";
    case command::text_redo: return "text.redo";
    case command::gui_widget_next: return "gui.widget.next";
    case command::gui_widget_prev: return "gui.widget.prev";
    case command::gui_activate: return "gui.activate";
    default:
        tt_no_default;
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
    if (name == "text.cursor.char.left") {
        return command::text_cursor_char_left;
    } else if (name == "text.cursor.char.right") {
        return command::text_cursor_char_right;
    } else if (name == "text.cursor.word.left") {
        return command::text_cursor_word_left;
    } else if (name == "text.cursor.word.right") {
        return command::text_cursor_word_right;
    } else if (name == "text.cursor.line.begin") {
        return command::text_cursor_line_begin;
    } else if (name == "text.cursor.line.end") {
        return command::text_cursor_line_end;
    } else if (name == "text.select.char.left") {
        return command::text_select_char_left;
    } else if (name == "text.select.char.right") {
        return command::text_select_char_right;
    } else if (name == "text.select.word") {
        return command::text_select_word;
    } else if (name == "text.select.word.left") {
        return command::text_select_word_left;
    } else if (name == "text.select.word.right") {
        return command::text_select_word_right;
    } else if (name == "text.select.line.begin") {
        return command::text_select_line_begin;
    } else if (name == "text.select.line.end") {
        return command::text_select_line_end;
    } else if (name == "text.select.document") {
        return command::text_select_document;
    } else if (name == "text.mode.insert") {
        return command::text_mode_insert;
    } else if (name == "text.delete.char.prev") {
        return command::text_delete_char_prev;
    } else if (name == "text.delete.char.next") {
        return command::text_delete_char_next;
    } else if (name == "text.edit.paste") {
        return command::text_edit_paste;
    } else if (name == "text.edit.copy") {
        return command::text_edit_copy;
    } else if (name == "text.edit.cut") {
        return command::text_edit_cut;
    } else if (name == "text.undo") {
        return command::text_undo;
    } else if (name == "text.redo") {
        return command::text_redo;
    } else if (name == "gui.widget.next") {
        return command::gui_widget_next;
    } else if (name == "gui.widget.prev") {
        return command::gui_widget_prev;
    } else if (name == "gui.activate") {
        return command::gui_activate;
    } else {
        return command::unknown;
    }
}

}
