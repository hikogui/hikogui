// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/mouse_event.hpp"
#include "../unicode/unicode_bidi.hpp"

namespace tt::inline v1 {

text_widget::text_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    text.subscribe(_reconstrain_callback);
}

widget_constraints const &text_widget::set_constraints() noexcept
{
    _layout = {};

    _shaped_text = text_shaper{font_book(), *text, theme().text_style(*text_style)};
    ttlet[shaped_text_rectangle, cap_height] =
        _shaped_text.bounding_rectangle(std::numeric_limits<float>::infinity(), alignment->vertical());
    _shaped_text_cap_height = cap_height;
    ttlet shaped_text_size = shaped_text_rectangle.size();

    _selection.clear_selection(_shaped_text.size());

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

    scroll_to_show_selection();
}

void text_widget::scroll_to_show_selection() noexcept
{
    if (visible and focus) {
        ttlet cursor = _selection.cursor();
        ttlet char_it = _shaped_text.begin() + cursor.index();
        if (char_it < _shaped_text.end()) {
            scroll_to_show(char_it->rectangle);
        }
    }
}

void text_widget::draw(draw_context const &context) noexcept
{
    if (_last_drag_mouse_event) {
        if (_last_drag_mouse_event_next_repeat == utc_nanoseconds{}) {
            _last_drag_mouse_event_next_repeat = context.display_time_point + _last_drag_mouse_event_repeat_interval;

        } else if (context.display_time_point >= _last_drag_mouse_event_next_repeat) {
            _last_drag_mouse_event_next_repeat = context.display_time_point + _last_drag_mouse_event_repeat_interval;

            // The last drag mouse event was stored in window coordinate to compensate for scrolling, translate it
            // back to local coordinates before handling the mouse event again.
            auto new_mouse_event = _last_drag_mouse_event;
            new_mouse_event.position = point2{_layout.from_window * _last_drag_mouse_event.position};

            // When mouse is dragging a selection, start continues redraw and scroll parent views to display the selection.
            text_widget::handle_event(new_mouse_event);
            scroll_to_show_selection();
        }
        request_redraw();
    }

    if (visible and overlaps(context, layout())) {
        context.draw_text(layout(), _shaped_text);

        context.draw_text_selection(layout(), _shaped_text, _selection, theme().color(theme_color::text_select));

        if (enabled and focus) {
            context.draw_text_cursors(
                layout(),
                _shaped_text,
                _selection.cursor(),
                theme().color(theme_color::primary_cursor),
                theme().color(theme_color::secondary_cursor),
                _overwrite_mode,
                static_cast<bool>(_has_dead_character));
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
    ttlet &text_proxy = text.cget();
    ttlet[first, last] = _selection.selection_indices();

    return gstring_view{*text_proxy}.substr(first, last - first);
}

void text_widget::fix_cursor_position(size_t size) noexcept
{
    if (_overwrite_mode and _selection.empty() and _selection.cursor().after()) {
        _selection = _selection.cursor().before_neighbor(size);
    }
}

void text_widget::replace_selection(gstring replacement) noexcept
{
    undo_push();

    auto text_proxy = text.get();
    ttlet[first, last] = _selection.selection_indices();
    text_proxy->replace(first, last - first, replacement);

    _selection = text_cursor{first + replacement.size() - 1, true, text_proxy->size()};
    fix_cursor_position(text_proxy->size());
}

void text_widget::add_character(grapheme c, add_type mode) noexcept
{
    ttlet original_cursor = _selection.cursor();
    auto original_grapheme = grapheme{char32_t{0xffff}};

    if (_selection.empty() and _overwrite_mode and original_cursor.before()) {
        original_grapheme = (*text.cget())[original_cursor.index()];

        ttlet[first, last] = _shaped_text.select_char(original_cursor);
        _selection.drag_selection(last);
    }
    replace_selection(gstring{c});

    if (mode == add_type::insert) {
        // The character was inserted, put the cursor back where it was.
        _selection = original_cursor;

    } else if (mode == add_type::dead) {
        _selection = original_cursor.before_neighbor(text.cget()->size());
        _has_dead_character = original_grapheme;
    }
}

void text_widget::delete_dead_character() noexcept
{
    if (_has_dead_character) {
        tt_axiom(_selection.cursor().before());
        tt_axiom(_selection.cursor().index() < text.cget()->size());
        if (_has_dead_character.valid()) {
            (*text.get())[_selection.cursor().index()] = _has_dead_character;
        } else {
            text->erase(_selection.cursor().index(), 1);
        }
    }
    _has_dead_character.clear();
}

void text_widget::delete_character_next() noexcept
{
    if (_selection.empty()) {
        auto cursor = _selection.cursor();
        cursor = cursor.before_neighbor(_shaped_text.size());

        ttlet[first, last] = _shaped_text.select_char(cursor);
        _selection.drag_selection(last);
    }

    return replace_selection(gstring{});
}

void text_widget::delete_character_prev() noexcept
{
    if (_selection.empty()) {
        auto cursor = _selection.cursor();
        cursor = cursor.after_neighbor(_shaped_text.size());

        ttlet[first, last] = _shaped_text.select_char(cursor);
        _selection.drag_selection(first);
    }

    return replace_selection(gstring{});
}

void text_widget::delete_word_next() noexcept
{
    if (_selection.empty()) {
        auto cursor = _selection.cursor();
        cursor = cursor.before_neighbor(_shaped_text.size());

        ttlet[first, last] = _shaped_text.select_word(cursor);
        _selection.drag_selection(last);
    }

    return replace_selection(gstring{});
}

void text_widget::delete_word_prev() noexcept
{
    if (_selection.empty()) {
        auto cursor = _selection.cursor();
        cursor = cursor.after_neighbor(_shaped_text.size());

        ttlet[first, last] = _shaped_text.select_word(cursor);
        _selection.drag_selection(first);
    }

    return replace_selection(gstring{});
}

void text_widget::reset_state(char const *states) noexcept
{
    while (*states != 0) {
        switch (*states) {
        case 'D': delete_dead_character(); break;
        case 'X': _vertical_movement_x = std::numeric_limits<float>::quiet_NaN(); break;
        default: tt_no_default();
        }
        ++states;
    }
}

bool text_widget::handle_event(tt::command command) noexcept
{
    tt_axiom(is_gui_thread());
    request_relayout();

    if (enabled) {
        switch (command) {
        case command::text_mode_insert:
            reset_state("DX");
            _overwrite_mode = not _overwrite_mode;
            fix_cursor_position();
            return true;

        case command::text_edit_paste:
            reset_state("DX");
            if (edit_mode == edit_mode_type::line_editable) {
                replace_selection(to_gstring(window.get_text_from_clipboard(), U' '));
            } else {
                replace_selection(to_gstring(window.get_text_from_clipboard()));
            }
            return true;

        case command::text_edit_copy:
            reset_state("DX");
            if (ttlet selected_text_ = selected_text(); not selected_text_.empty()) {
                window.set_text_on_clipboard(to_string(selected_text_));
            }
            return true;

        case command::text_edit_cut:
            reset_state("DX");
            window.set_text_on_clipboard(to_string(selected_text()));
            replace_selection(gstring{});
            return true;

        case command::text_undo:
            reset_state("DX");
            undo();
            return true;

        case command::text_redo:
            reset_state("DX");
            redo();
            return true;

        case command::text_insert_line:
            if (edit_mode == edit_mode_type::fully_editable) {
                reset_state("DX");
                add_character(grapheme{unicode_PS}, add_type::append);
                return true;
            }
            break;

        case command::text_insert_line_up:
            if (edit_mode == edit_mode_type::fully_editable) {
                reset_state("DX");
                _selection = _shaped_text.move_begin_paragraph(_selection.cursor());
                add_character(grapheme{unicode_PS}, add_type::insert);
                return true;
            }
            break;

        case command::text_insert_line_down:
            if (edit_mode == edit_mode_type::fully_editable) {
                reset_state("DX");
                _selection = _shaped_text.move_end_paragraph(_selection.cursor());
                add_character(grapheme{unicode_PS}, add_type::insert);
                return true;
            }
            break;

        case command::text_delete_char_next:
            reset_state("DX");
            delete_character_next();
            return true;

        case command::text_delete_char_prev:
            reset_state("DX");
            delete_character_prev();
            return true;

        case command::text_delete_word_next:
            reset_state("DX");
            delete_word_next();
            return true;

        case command::text_delete_word_prev:
            reset_state("DX");
            delete_word_prev();
            return true;

        case command::text_cursor_left_char:
            reset_state("DX");
            _selection = _shaped_text.move_left_char(_selection.cursor(), _overwrite_mode);
            return true;

        case command::text_cursor_right_char:
            reset_state("DX");
            _selection = _shaped_text.move_right_char(_selection.cursor(), _overwrite_mode);
            return true;

        case command::text_cursor_down_char:
            reset_state("D");
            _selection = _shaped_text.move_down_char(_selection.cursor(), _vertical_movement_x);
            return true;

        case command::text_cursor_up_char:
            reset_state("D");
            _selection = _shaped_text.move_up_char(_selection.cursor(), _vertical_movement_x);
            return true;

        case command::text_cursor_left_word:
            reset_state("DX");
            _selection = _shaped_text.move_left_word(_selection.cursor(), _overwrite_mode);
            return true;

        case command::text_cursor_right_word:
            reset_state("DX");
            _selection = _shaped_text.move_right_word(_selection.cursor(), _overwrite_mode);
            return true;

        case command::text_cursor_begin_line:
            reset_state("DX");
            _selection = _shaped_text.move_begin_line(_selection.cursor());
            return true;

        case command::text_cursor_end_line:
            reset_state("DX");
            _selection = _shaped_text.move_end_line(_selection.cursor());
            return true;

        case command::text_cursor_begin_sentence:
            reset_state("DX");
            _selection = _shaped_text.move_begin_sentence(_selection.cursor());
            return true;

        case command::text_cursor_end_sentence:
            reset_state("DX");
            _selection = _shaped_text.move_end_sentence(_selection.cursor());
            return true;

        case command::text_cursor_begin_document:
            reset_state("DX");
            _selection = _shaped_text.move_begin_document(_selection.cursor());
            return true;

        case command::text_cursor_end_document:
            reset_state("DX");
            _selection = _shaped_text.move_end_document(_selection.cursor());
            return true;

        case command::gui_cancel:
            reset_state("DX");
            _selection.clear_selection(_shaped_text.size());
            return true;

        case command::text_select_left_char:
            reset_state("DX");
            _selection.drag_selection(_shaped_text.move_left_char(_selection.cursor(), false));
            return true;

        case command::text_select_right_char:
            reset_state("DX");
            _selection.drag_selection(_shaped_text.move_right_char(_selection.cursor(), false));
            return true;

        case command::text_select_down_char:
            reset_state("D");
            _selection.drag_selection(_shaped_text.move_down_char(_selection.cursor(), _vertical_movement_x));
            return true;

        case command::text_select_up_char:
            reset_state("D");
            _selection.drag_selection(_shaped_text.move_up_char(_selection.cursor(), _vertical_movement_x));
            return true;

        case command::text_select_left_word:
            reset_state("DX");
            _selection.drag_selection(_shaped_text.move_left_word(_selection.cursor(), false));
            return true;

        case command::text_select_right_word:
            reset_state("DX");
            _selection.drag_selection(_shaped_text.move_right_word(_selection.cursor(), false));
            return true;

        case command::text_select_begin_line:
            reset_state("DX");
            _selection.drag_selection(_shaped_text.move_begin_line(_selection.cursor()));
            return true;

        case command::text_select_end_line:
            reset_state("DX");
            _selection.drag_selection(_shaped_text.move_end_line(_selection.cursor()));
            return true;

        case command::text_select_begin_sentence:
            reset_state("DX");
            _selection.drag_selection(_shaped_text.move_begin_sentence(_selection.cursor()));
            return true;

        case command::text_select_end_sentence:
            reset_state("DX");
            _selection.drag_selection(_shaped_text.move_end_sentence(_selection.cursor()));
            return true;

        case command::text_select_begin_document:
            reset_state("DX");
            _selection.drag_selection(_shaped_text.move_begin_document(_selection.cursor()));
            return true;

        case command::text_select_end_document:
            reset_state("DX");
            _selection.drag_selection(_shaped_text.move_end_document(_selection.cursor()));
            return true;

        case command::text_select_document:
            reset_state("DX");
            _selection = _shaped_text.move_begin_document(_selection.cursor());
            _selection.drag_selection(_shaped_text.move_end_document(_selection.cursor()));
            return true;

        default:;
        }
    }

    return super::handle_event(command);
}

bool text_widget::handle_event(keyboard_event const &event) noexcept
{
    using enum keyboard_event::Type;

    tt_axiom(is_gui_thread());
    request_relayout();

    auto handled = super::handle_event(event);

    if (enabled) {
        switch (event.type) {
        case grapheme:
            reset_state("DX");
            add_character(event.grapheme, add_type::append);
            return true;

        case partial_grapheme:
            reset_state("DX");
            add_character(event.grapheme, add_type::dead);
            return true;

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

        ttlet cursor = _shaped_text.get_nearest_cursor(event.position);

        switch (event.type) {
            using enum mouse_event::Type;
        case ButtonUp:
            // Stop the continues redrawing during dragging.
            // Also reset the time, so on drag-start it will initialize the time, which will
            // cause a smooth startup of repeating.
            _last_drag_mouse_event = {};
            _last_drag_mouse_event_next_repeat = {};
            break;

        case ButtonDown:
            switch (event.clickCount) {
            case 1:
                reset_state("DX");
                _selection = cursor;
                break;
            case 2:
                reset_state("DX");
                _selection.start_selection(cursor, _shaped_text.select_word(cursor));
                break;
            case 3:
                reset_state("DX");
                _selection.start_selection(cursor, _shaped_text.select_sentence(cursor));
                break;
            case 4:
                reset_state("DX");
                _selection.start_selection(cursor, _shaped_text.select_paragraph(cursor));
                break;
            case 5:
                reset_state("DX");
                _selection.start_selection(cursor, _shaped_text.select_document(cursor));
                break;
            default:;
            }

            request_redraw();
            break;

        case Drag:
            switch (event.clickCount) {
            case 1:
                reset_state("DX");
                _selection.drag_selection(cursor);
                break;
            case 2:
                reset_state("DX");
                _selection.drag_selection(cursor, _shaped_text.select_word(cursor));
                break;
            case 3:
                reset_state("DX");
                _selection.drag_selection(cursor, _shaped_text.select_sentence(cursor));
                break;
            case 4:
                reset_state("DX");
                _selection.drag_selection(cursor, _shaped_text.select_paragraph(cursor));
                break;
            default:;
            }

            // Drag events must be repeated, so that dragging is continues when it causes scrolling.
            // Normally mouse positions are kept in the local coordinate system, but scrolling
            // causes this coordinate system to shift, so translate it to the window coordinate system here.
            _last_drag_mouse_event = event;
            _last_drag_mouse_event.position = point2{_layout.to_window * event.position};
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

    if (visible and enabled and layout().contains(position)) {
        switch (*edit_mode) {
        case edit_mode_type::selectable: return hitbox{this, position, hitbox::Type::Default};
        case edit_mode_type::line_editable: return hitbox{this, position, hitbox::Type::TextEdit};
        case edit_mode_type::fully_editable: return hitbox{this, position, hitbox::Type::TextEdit};
        default: return hitbox{};
        }
    } else {
        return hitbox{};
    }
}

[[nodiscard]] bool text_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    using enum edit_mode_type;
    using enum keyboard_focus_group;

    return visible and enabled and any(group & normal) and (edit_mode == line_editable or edit_mode == fully_editable);
}

} // namespace tt::inline v1
