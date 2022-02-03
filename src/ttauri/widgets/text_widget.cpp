// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/mouse_event.hpp"

namespace tt::inline v1 {

text_widget::text_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    text.subscribe(_reconstrain_callback);
}

widget_constraints const &text_widget::set_constraints() noexcept
{
    _layout = {};

    ttlet text_ = text.cget();
    _selection.clear_selection(text_->size());
    _shaped_text = text_shaper{font_book(), text_, theme().text_style(*text_style)};
    ttlet[shaped_text_rectangle, cap_height] = _shaped_text.bounding_rectangle(500.0f, alignment->vertical());
    _shaped_text_cap_height = cap_height;
    ttlet shaped_text_size = shaped_text_rectangle.size();

    return _constraints = {shaped_text_size, shaped_text_size, shaped_text_size, theme().margin};
}

void text_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        _shaped_text.layout(
            layout.rectangle(),
            layout.base_line() - _shaped_text_cap_height * 0.5f,
            layout.sub_pixel_size,
            layout.writing_direction,
            *alignment);
    }
}

void text_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        context.draw_text(layout(), _shaped_text);

        context.draw_text_selection(layout(), _shaped_text, _selection, theme().color(theme_color::text_select));

        if (enabled and focus) {
            context.draw_text_cursors(
                layout(),
                _shaped_text,
                _selection.cursor(),
                theme().color(theme_color::cursor),
                theme().color(theme_color::incomplete_glyph), _insertion_mode);
        }
    }
}

void text_widget::undo_push() noexcept
{
    _undo_stack.emplace(*text.cget(), _selection);
}

void text_widget::undo() noexcept
{
    if (_undo_stack.can_undo()) {
        ttlet &tmp = _undo_stack.undo(*text.cget(), _selection);
        text = tmp.text;
        _selection = tmp.selection;
    }
}

void text_widget::redo() noexcept
{
    if (_undo_stack.can_redo()) {
        ttlet &tmp = _undo_stack.redo();
        text = tmp.text;
        _selection = tmp.selection;
    }
}

[[nodiscard]] gstring_view text_widget::selected_text() const noexcept
{
    ttlet[first, last] = _selection.selection_indices();
    ttlet &text_ = *text.cget();
    tt_axiom(first <= last and last <= text_.size());

    return gstring_view{text_}.substr(first, last - first);
}

void text_widget::replace_selection(gstring replacement) noexcept
{
    undo_push();

    ttlet[first, last] = _selection.selection_indices();
    auto text_proxy = text.get();
    text_proxy->replace(first, last - first, replacement);
    _selection.set_cursor(text_cursor{first, false}.advance_char(narrow_cast<ptrdiff_t>(replacement.size()), text_proxy->size()));
}

void text_widget::add_char(grapheme c) noexcept
{
    if (not _selection.empty()) {
        return replace_selection(gstring{c});
    }

    undo_push();

    auto cursor = _selection.cursor();
    auto index = cursor.after() ? cursor.index() + 1 : cursor.index();

    auto text_proxy = text.get();
    if (_insertion_mode) {
        text_proxy->insert(index, 1, c);
    } else {
        text_proxy[index] = c;
    }
    _selection.set_cursor(cursor.advance_char(1, text_proxy->size()));
}

void text_widget::delete_char_prev() noexcept
{
    auto cursor = _selection.cursor();

    if (not _selection.empty()) {
        return replace_selection(gstring{});
    } else if (cursor.start_of_text()) {
        return;
    }

    undo_push();

    // Place the cursor after the character to delete.
    if (cursor.before()) {
        cursor = cursor.neighbour();
    }

    // Delete the character.
    auto text_proxy = text.get();
    tt_axiom(cursor.index() < text_proxy->size());
    text_proxy->erase(cursor.index(), 1);

    _selection.set_cursor(cursor.advance_char(-1, text_proxy->size()));
}

void text_widget::delete_char_next() noexcept
{
    auto cursor = _selection.cursor();

    if (not _selection.empty()) {
        return replace_selection(gstring{});
    } else if (cursor.end_of_text(text->size())) {
        return;
    }

    undo_push();

    // Place the cursor before the character to delete.
    if (cursor.after()) {
        cursor = cursor.neighbour();
    }

    // Remove the character.
    auto text_proxy = text.get();
    tt_axiom(cursor.index() < text_proxy->size());
    text_proxy->erase(cursor.index(), 1);

    // Place the cursor after the previous character, unless already at start of text.
    if (not cursor.start_of_text()) {
        cursor = cursor.neighbour();
    }
    _selection.set_cursor(cursor);
}

bool text_widget::handle_event(tt::command command) noexcept
{
    tt_axiom(is_gui_thread());
    request_relayout();

    // On commands other than vertical movement, reset the vertical movement state.
    if (is_text_edit_command(command)) {
        switch (command) {
        case command::text_cursor_up_char:
        case command::text_cursor_down_char:
        case command::text_select_up_char:
        case command::text_select_down_char: break;
        default: _vertical_movement_x = std::numeric_limits<float>::quiet_NaN();
        }
    }

    if (enabled) {
        // clang-format off
        switch (command) {
        case command::text_mode_insert:
            _insertion_mode = not _insertion_mode;
            return true;

        case command::text_edit_paste:
            replace_selection(to_gstring(window.get_text_from_clipboard()));
            return true;
        
        case command::text_edit_copy:
            if (ttlet selected_text_ = selected_text(); not selected_text_.empty()) {
                window.set_text_on_clipboard(to_string(selected_text_));
            }
            return true;

        case command::text_edit_cut:
            window.set_text_on_clipboard(to_string(selected_text()));
            replace_selection(gstring{});
            return true;

        case command::text_undo: undo(); return true;
        case command::text_redo: redo(); return true;

        case command::text_delete_char_next: delete_char_next(); return true;
        case command::text_delete_char_prev: delete_char_prev(); return true;

        case command::text_cursor_left_char:
            _selection.set_cursor(_shaped_text.move_left_char(_selection.cursor())); return true;
        case command::text_cursor_right_char:
            _selection.set_cursor(_shaped_text.move_right_char(_selection.cursor())); return true;
        case command::text_cursor_down_char:
            _selection.set_cursor(_shaped_text.move_down_char(_selection.cursor(), _vertical_movement_x)); return true;
        case command::text_cursor_up_char:
            _selection.set_cursor(_shaped_text.move_up_char(_selection.cursor(), _vertical_movement_x)); return true;
        case command::text_cursor_left_word:
            _selection.set_cursor(_shaped_text.move_left_word(_selection.cursor())); return true;
        case command::text_cursor_right_word:
            _selection.set_cursor(_shaped_text.move_right_word(_selection.cursor())); return true;
        case command::text_cursor_begin_line:
            _selection.set_cursor(_shaped_text.move_begin_line(_selection.cursor())); return true;
        case command::text_cursor_end_line:
            _selection.set_cursor(_shaped_text.move_end_line(_selection.cursor())); return true;
        case command::text_cursor_begin_sentence:
            _selection.set_cursor(_shaped_text.move_begin_sentence(_selection.cursor())); return true;
        case command::text_cursor_end_sentence:
            _selection.set_cursor(_shaped_text.move_end_sentence(_selection.cursor())); return true;
        case command::text_cursor_begin_document:
            _selection.set_cursor(_shaped_text.move_begin_document(_selection.cursor())); return true;
        case command::text_cursor_end_document:
            _selection.set_cursor(_shaped_text.move_end_document(_selection.cursor())); return true;

        case command::gui_cancel:
            _selection.clear_selection(); return true;
        case command::text_select_left_char:
            _selection.drag_selection(_shaped_text.move_left_char(_selection.cursor())); return true;
        case command::text_select_right_char:
            _selection.drag_selection(_shaped_text.move_right_char(_selection.cursor())); return true;
        case command::text_select_down_char:
            _selection.drag_selection(_shaped_text.move_down_char(_selection.cursor(), _vertical_movement_x)); return true;
        case command::text_select_up_char:
            _selection.drag_selection(_shaped_text.move_up_char(_selection.cursor(), _vertical_movement_x)); return true;
        case command::text_select_left_word:
            _selection.drag_selection(_shaped_text.move_left_word(_selection.cursor())); return true;
        case command::text_select_right_word:
            _selection.drag_selection(_shaped_text.move_right_word(_selection.cursor())); return true;
        case command::text_select_begin_line:
            _selection.drag_selection(_shaped_text.move_begin_line(_selection.cursor())); return true;
        case command::text_select_end_line:
            _selection.drag_selection(_shaped_text.move_end_line(_selection.cursor())); return true;
        case command::text_select_begin_sentence:
            _selection.drag_selection(_shaped_text.move_begin_sentence(_selection.cursor())); return true;
        case command::text_select_end_sentence:
            _selection.drag_selection(_shaped_text.move_end_sentence(_selection.cursor())); return true;
        case command::text_select_begin_document:
            _selection.drag_selection(_shaped_text.move_begin_document(_selection.cursor())); return true;
        case command::text_select_end_document:
            _selection.drag_selection(_shaped_text.move_end_document(_selection.cursor())); return true;

        default:;
        }
        // clang-format on
    }

    return super::handle_event(command);
}

bool text_widget::handle_event(keyboard_event const &event) noexcept
{
    using enum keyboard_event::Type;

    tt_axiom(is_gui_thread());

    auto handled = super::handle_event(event);

    if (enabled) {
        switch (event.type) {
        case grapheme:
            add_char(event.grapheme);
            return true;

            // case Partialgrapheme:
            //     handled = true;
            //     _field.insert_partial_grapheme(event.grapheme);
            //     commit(false);
            //     break;

        default:;
        }
    }

    return handled;
}

bool text_widget::handle_event(mouse_event const &event) noexcept
{
    tt_axiom(is_gui_thread());
    auto handled = super::handle_event(event);
    if (edit_mode == edit_mode_type::fixed) {
        return handled;
    }

    if (event.cause.leftButton) {
        handled = true;

        if (not *enabled) {
            return true;
        }

        ttlet cursor = _shaped_text.get_nearest(event.position);
        _vertical_movement_x = event.position.x();

        switch (event.type) {
            using enum mouse_event::Type;
        case ButtonDown:
            switch (event.clickCount) {
            case 1: _selection.set_cursor(cursor); break;
            case 2: _selection.start_selection(cursor, _shaped_text.get_word(cursor)); break;
            case 3: _selection.start_selection(cursor, _shaped_text.get_sentence(cursor)); break;
            default:;
            }

            // Record the last time the cursor is moved, so that the caret remains lit.
            //_last_update_time_point = event.timePoint;

            request_redraw();
            break;

        case Drag:
            switch (event.clickCount) {
            case 1: _selection.drag_selection(cursor); break;
            case 2: _selection.drag_selection(cursor, _shaped_text.get_word(cursor)); break;
            case 3: _selection.drag_selection(cursor, _shaped_text.get_sentence(cursor)); break;
            default:;
            }

            request_redraw();
            break;

        default:;
        }
    }
    return handled;
}

hitbox text_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and enabled and edit_mode != edit_mode_type::fixed and layout().contains(position)) {
        return hitbox{this, position, edit_mode == edit_mode_type::editable ? hitbox::Type::TextEdit : hitbox::Type::Default};
    } else {
        return hitbox{};
    }
}

[[nodiscard]] bool text_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    return visible and enabled and edit_mode == edit_mode_type::editable and any(group & tt::keyboard_focus_group::normal);
}

} // namespace tt::inline v1
