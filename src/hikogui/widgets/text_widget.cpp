// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"
#include "../os_settings.hpp"
#include "../scoped_task.hpp"
#include "../when_any.hpp"
#include "../GUI/gui_window.hpp"
#include "../unicode/unicode_bidi.hpp"

namespace hi::inline v1 {

text_widget::text_widget(gui_window& window, widget *parent) noexcept : super(window, parent)
{
    _text_cbt = text.subscribe([&](auto...) {
        update_shaped_text();
        request_reconstrain();
    });
    _text_style_cbt = text_style.subscribe([&](auto...) {
        update_shaped_text();
        request_reconstrain();
    });

    _cursor_state_cbt = _cursor_state.subscribe([&](auto...){ request_redraw(); });

    _blink_cursor = blink_cursor();
}

void text_widget::update_shaped_text() noexcept
{
    _selection.resize(text->size());
    _shaped_text = text_shaper{font_book(), *text, theme().text_style(*text_style), theme().scale};
}

widget_constraints const& text_widget::set_constraints() noexcept
{
    _layout = {};

    update_shaped_text();
    hilet[shaped_text_rectangle, cap_height] =
        _shaped_text.bounding_rectangle(std::numeric_limits<float>::infinity(), alignment->vertical());
    _shaped_text_cap_height = cap_height;
    hilet shaped_text_size = shaped_text_rectangle.size();

    if (*edit_mode == edit_mode_type::line_editable) {
        // In line-edit mode the text should not wrap.
        return _constraints = {shaped_text_size, shaped_text_size, shaped_text_size, theme().margin};
    } else {
        // Allow the text to be 550.0f pixels wide.
        hilet[preferred_shaped_text_rectangle, dummy] = _shaped_text.bounding_rectangle(550.0f, alignment->vertical());
        hilet preferred_shaped_text_size = preferred_shaped_text_rectangle.size();

        hilet height = std::max(shaped_text_size.height(), preferred_shaped_text_size.height());
        return _constraints = {
                   extent2{preferred_shaped_text_size.width(), height},
                   extent2{preferred_shaped_text_size.width(), height},
                   extent2{shaped_text_size.width(), height},
                   theme().margin};
    }
}

void text_widget::set_layout(widget_layout const& layout) noexcept
{
    if (compare_store(_layout, layout)) {
        // clang-format off
        _base_line =
            *alignment == vertical_alignment::bottom ? layout.rectangle().bottom() :
            *alignment == vertical_alignment::middle ? layout.rectangle().middle() - _shaped_text_cap_height * 0.5f :
            layout.rectangle().top() - _shaped_text_cap_height;
        // clang-format on

        _shaped_text.layout(layout.rectangle(), _base_line, layout.sub_pixel_size, layout.writing_direction, *alignment);

        // Update scroll position every time the text or layout has changed.
        _request_scroll = true;
    }
}

void text_widget::scroll_to_show_selection() noexcept
{
    if (*visible and *focus) {
        hilet cursor = _selection.cursor();
        hilet char_it = _shaped_text.begin() + cursor.index();
        if (char_it < _shaped_text.end()) {
            scroll_to_show(char_it->rectangle);
        }
    }
}

void text_widget::request_scroll() noexcept
{
    // At a minimum we need to request a redraw so that
    // `scroll_to_show_selection()` is called on the next frame.
    _request_scroll = true;
    request_redraw();
}

scoped_task<> text_widget::blink_cursor() noexcept
{
    while (true) {
        if (*visible and *enabled and *focus) {
            switch (*_cursor_state) {
            case cursor_state_type::busy:
                _cursor_state = cursor_state_type::on;
                co_await when_any(os_settings::cursor_blink_delay(), visible, enabled, focus);
                break;

            case cursor_state_type::on:
                _cursor_state = cursor_state_type::off;
                co_await when_any(os_settings::cursor_blink_interval() / 2, visible, enabled, focus);
                break;

            case cursor_state_type::off:
                _cursor_state = cursor_state_type::on;
                co_await when_any(os_settings::cursor_blink_interval() / 2, visible, enabled, focus);
                break;

            default:
                _cursor_state = cursor_state_type::busy;
            }

        } else {
            _cursor_state = cursor_state_type::none;
            co_await when_any(visible, enabled, focus);
        }
    }
}

void text_widget::draw(draw_context const& context) noexcept
{
    using namespace std::literals::chrono_literals;

    // After potential reconstrain and relayout, updating the shaped-text, ask the parent window to scroll if needed.
    if (std::exchange(_request_scroll, false)) {
        scroll_to_show_selection();
    }

    if (_last_drag_mouse_event) {
        if (_last_drag_mouse_event_next_repeat == utc_nanoseconds{}) {
            _last_drag_mouse_event_next_repeat = context.display_time_point + os_settings::keyboard_repeat_delay();

        } else if (context.display_time_point >= _last_drag_mouse_event_next_repeat) {
            _last_drag_mouse_event_next_repeat = context.display_time_point + os_settings::keyboard_repeat_interval();

            // The last drag mouse event was stored in window coordinate to compensate for scrolling, translate it
            // back to local coordinates before handling the mouse event again.
            auto new_mouse_event = _last_drag_mouse_event;
            new_mouse_event.mouse().position = point2{_layout.from_window * _last_drag_mouse_event.mouse().position};

            // When mouse is dragging a selection, start continues redraw and scroll parent views to display the selection.
            text_widget::handle_event(new_mouse_event);
        }
        scroll_to_show_selection();
        request_redraw();
    }

    if (*visible and overlaps(context, layout())) {
        context.draw_text(layout(), _shaped_text);

        context.draw_text_selection(layout(), _shaped_text, _selection, theme().color(semantic_color::text_select));

        if (*_cursor_state == cursor_state_type::on or *_cursor_state == cursor_state_type::busy) {
            context.draw_text_cursors(
                layout(),
                _shaped_text,
                _selection.cursor(),
                theme().color(semantic_color::primary_cursor),
                theme().color(semantic_color::secondary_cursor),
                _overwrite_mode,
                static_cast<bool>(_has_dead_character));
        }
    }
}

void text_widget::undo_push() noexcept
{
    _undo_stack.emplace(*text, _selection);
}

void text_widget::undo() noexcept
{
    if (_undo_stack.can_undo()) {
        hilet& tmp = _undo_stack.undo(*text, _selection);
        text = tmp.text;
        _selection = tmp.selection;
    }
}

void text_widget::redo() noexcept
{
    if (_undo_stack.can_redo()) {
        hilet& tmp = _undo_stack.redo();
        text = tmp.text;
        _selection = tmp.selection;
    }
}

[[nodiscard]] gstring_view text_widget::selected_text() const noexcept
{
    hilet[first, last] = _selection.selection_indices();

    return gstring_view{*text}.substr(first, last - first);
}

void text_widget::fix_cursor_position() noexcept
{
    hilet size = text->size();
    if (_overwrite_mode and _selection.empty() and _selection.cursor().after()) {
        _selection = _selection.cursor().before_neighbor(size);
    }
    _selection.resize(size);
}

void text_widget::replace_selection(gstring const& replacement) noexcept
{
    undo_push();

    hilet[first, last] = _selection.selection_indices();
    text.proxy()->replace(first, last - first, replacement);

    _selection = text_cursor{first + replacement.size() - 1, true};
    fix_cursor_position();
}

void text_widget::add_character(grapheme c, add_type mode) noexcept
{
    hilet original_cursor = _selection.cursor();
    auto original_grapheme = grapheme{char32_t{0xffff}};

    if (_selection.empty() and _overwrite_mode and original_cursor.before()) {
        original_grapheme = (*text)[original_cursor.index()];

        hilet[first, last] = _shaped_text.select_char(original_cursor);
        _selection.drag_selection(last);
    }
    replace_selection(gstring{c});

    if (mode == add_type::insert) {
        // The character was inserted, put the cursor back where it was.
        _selection = original_cursor;

    } else if (mode == add_type::dead) {
        _selection = original_cursor.before_neighbor(text->size());
        _has_dead_character = original_grapheme;
    }
}

void text_widget::delete_dead_character() noexcept
{
    if (_has_dead_character) {
        hi_axiom(_selection.cursor().before());
        hi_axiom(_selection.cursor().index() < text->size());
        if (_has_dead_character.valid()) {
            (*text.proxy())[_selection.cursor().index()] = _has_dead_character;
        } else {
            text.proxy()->erase(_selection.cursor().index(), 1);
        }
    }
    _has_dead_character.clear();
}

void text_widget::delete_character_next() noexcept
{
    if (_selection.empty()) {
        auto cursor = _selection.cursor();
        cursor = cursor.before_neighbor(_shaped_text.size());

        hilet[first, last] = _shaped_text.select_char(cursor);
        _selection.drag_selection(last);
    }

    return replace_selection(gstring{});
}

void text_widget::delete_character_prev() noexcept
{
    if (_selection.empty()) {
        auto cursor = _selection.cursor();
        cursor = cursor.after_neighbor(_shaped_text.size());

        hilet[first, last] = _shaped_text.select_char(cursor);
        _selection.drag_selection(first);
    }

    return replace_selection(gstring{});
}

void text_widget::delete_word_next() noexcept
{
    if (_selection.empty()) {
        auto cursor = _selection.cursor();
        cursor = cursor.before_neighbor(_shaped_text.size());

        hilet[first, last] = _shaped_text.select_word(cursor);
        _selection.drag_selection(last);
    }

    return replace_selection(gstring{});
}

void text_widget::delete_word_prev() noexcept
{
    if (_selection.empty()) {
        auto cursor = _selection.cursor();
        cursor = cursor.after_neighbor(_shaped_text.size());

        hilet[first, last] = _shaped_text.select_word(cursor);
        _selection.drag_selection(first);
    }

    return replace_selection(gstring{});
}

void text_widget::reset_state(char const *states) noexcept
{
    while (*states != 0) {
        switch (*states) {
        case 'D':
            delete_dead_character();
            break;
        case 'X':
            _vertical_movement_x = std::numeric_limits<float>::quiet_NaN();
            break;
        case 'B':
            if (*_cursor_state == cursor_state_type::on or *_cursor_state == cursor_state_type::off) {
                _cursor_state = cursor_state_type::busy;
            }
            break;
        default:
            hi_no_default();
        }
        ++states;
    }
}

bool text_widget::handle_event(gui_event const& event) noexcept
{
    hi_axiom(is_gui_thread());

    switch (event.type()) {
        using enum edit_mode_type;
        using enum gui_event_type;

    case gui_widget_next:
    case gui_widget_prev:
    case keyboard_exit:
        // When the next widget is selected due to pressing the Tab key the text should be committed.
        // The `text_widget` does not handle gui_activate, so it will be forwarded to parent widgets,
        // such as `text_field_widget` which does.
        window.process_event(gui_event_type::gui_activate);
        return super::handle_event(event);

    case keyboard_grapheme:
        if (*enabled and *edit_mode >= line_editable) {
            reset_state("BDX");
            add_character(event.grapheme(), add_type::append);
            return true;
        }
        break;

    case keyboard_partial_grapheme:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            add_character(event.grapheme(), add_type::dead);
            return true;
        }
        break;

    case text_mode_insert:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _overwrite_mode = not _overwrite_mode;
            fix_cursor_position();
            return true;
        }
        break;

    case text_edit_paste:
        if (*enabled and *edit_mode == edit_mode_type::line_editable) {
            reset_state("BDX");
            replace_selection(to_gstring(window.get_text_from_clipboard(), U' '));
            return true;

        } else if (*enabled and *edit_mode == edit_mode_type::fully_editable) {
            reset_state("BDX");
            replace_selection(to_gstring(window.get_text_from_clipboard()));
            return true;
        }
        break;

    case text_edit_copy:
        if (*enabled and *edit_mode >= edit_mode_type::selectable) {
            reset_state("BDX");
            if (hilet selected_text_ = selected_text(); not selected_text_.empty()) {
                window.set_text_on_clipboard(to_string(selected_text_));
            }
            return true;
        }
        break;

    case text_edit_cut:
        if (*enabled and *edit_mode >= edit_mode_type::selectable) {
            reset_state("BDX");
            window.set_text_on_clipboard(to_string(selected_text()));
            if (*edit_mode >= edit_mode_type::line_editable) {
                replace_selection(gstring{});
            }
            return true;
        }
        break;

    case text_undo:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            undo();
            return true;
        }
        break;

    case text_redo:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            redo();
            return true;
        }
        break;

    case text_insert_line:
        if (*enabled and *edit_mode == edit_mode_type::fully_editable) {
            reset_state("BDX");
            add_character(grapheme{unicode_PS}, add_type::append);
            return true;
        }
        break;

    case text_insert_line_up:
        if (*enabled and *edit_mode == edit_mode_type::fully_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_begin_paragraph(_selection.cursor());
            add_character(grapheme{unicode_PS}, add_type::insert);
            return true;
        }
        break;

    case text_insert_line_down:
        if (*enabled and *edit_mode == edit_mode_type::fully_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_end_paragraph(_selection.cursor());
            add_character(grapheme{unicode_PS}, add_type::insert);
            return true;
        }
        break;

    case text_delete_char_next:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            delete_character_next();
            return true;
        }
        break;

    case text_delete_char_prev:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            delete_character_prev();
            return true;
        }
        break;

    case text_delete_word_next:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            delete_word_next();
            return true;
        }
        break;

    case text_delete_word_prev:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            delete_word_prev();
            return true;
        }
        break;

    case text_cursor_left_char:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_left_char(_selection.cursor(), _overwrite_mode);
            request_scroll();
            return true;
        }
        break;

    case text_cursor_right_char:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_right_char(_selection.cursor(), _overwrite_mode);
            request_scroll();
            return true;
        }
        break;

    case text_cursor_down_char:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BD");
            _selection = _shaped_text.move_down_char(_selection.cursor(), _vertical_movement_x);
            request_scroll();
            return true;
        }
        break;

    case text_cursor_up_char:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BD");
            _selection = _shaped_text.move_up_char(_selection.cursor(), _vertical_movement_x);
            request_scroll();
            return true;
        }
        break;

    case text_cursor_left_word:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_left_word(_selection.cursor(), _overwrite_mode);
            request_scroll();
            return true;
        }
        break;

    case text_cursor_right_word:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_right_word(_selection.cursor(), _overwrite_mode);
            request_scroll();
            return true;
        }
        break;

    case text_cursor_begin_line:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_begin_line(_selection.cursor());
            request_scroll();
            return true;
        }
        break;

    case text_cursor_end_line:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_end_line(_selection.cursor());
            request_scroll();
            return true;
        }
        break;

    case text_cursor_begin_sentence:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_begin_sentence(_selection.cursor());
            request_scroll();
            return true;
        }
        break;

    case text_cursor_end_sentence:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_end_sentence(_selection.cursor());
            request_scroll();
            return true;
        }
        break;

    case text_cursor_begin_document:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_begin_document(_selection.cursor());
            request_scroll();
            return true;
        }
        break;

    case text_cursor_end_document:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_end_document(_selection.cursor());
            request_scroll();
            return true;
        }
        break;

    case gui_cancel:
        if (*enabled and *edit_mode >= edit_mode_type::selectable) {
            reset_state("BDX");
            _selection.clear_selection(_shaped_text.size());
            return true;
        }
        break;

    case text_select_left_char:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection.drag_selection(_shaped_text.move_left_char(_selection.cursor(), false));
            request_scroll();
            return true;
        }
        break;

    case text_select_right_char:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection.drag_selection(_shaped_text.move_right_char(_selection.cursor(), false));
            request_scroll();
            return true;
        }
        break;

    case text_select_down_char:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BD");
            _selection.drag_selection(_shaped_text.move_down_char(_selection.cursor(), _vertical_movement_x));
            request_scroll();
            return true;
        }
        break;

    case text_select_up_char:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BD");
            _selection.drag_selection(_shaped_text.move_up_char(_selection.cursor(), _vertical_movement_x));
            request_scroll();
            return true;
        }
        break;

    case text_select_left_word:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection.drag_selection(_shaped_text.move_left_word(_selection.cursor(), false));
            request_scroll();
            return true;
        }
        break;

    case text_select_right_word:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection.drag_selection(_shaped_text.move_right_word(_selection.cursor(), false));
            request_scroll();
            return true;
        }
        break;

    case text_select_begin_line:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection.drag_selection(_shaped_text.move_begin_line(_selection.cursor()));
            request_scroll();
            return true;
        }
        break;

    case text_select_end_line:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection.drag_selection(_shaped_text.move_end_line(_selection.cursor()));
            request_scroll();
            return true;
        }
        break;

    case text_select_begin_sentence:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection.drag_selection(_shaped_text.move_begin_sentence(_selection.cursor()));
            request_scroll();
            return true;
        }
        break;

    case text_select_end_sentence:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection.drag_selection(_shaped_text.move_end_sentence(_selection.cursor()));
            request_scroll();
            return true;
        }
        break;

    case text_select_begin_document:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection.drag_selection(_shaped_text.move_begin_document(_selection.cursor()));
            request_scroll();
            return true;
        }
        break;

    case text_select_end_document:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection.drag_selection(_shaped_text.move_end_document(_selection.cursor()));
            request_scroll();
            return true;
        }
        break;

    case text_select_document:
        if (*enabled and *edit_mode >= edit_mode_type::line_editable) {
            reset_state("BDX");
            _selection = _shaped_text.move_begin_document(_selection.cursor());
            _selection.drag_selection(_shaped_text.move_end_document(_selection.cursor()));
            request_scroll();
            return true;
        }
        break;

    case mouse_up:
        if (*enabled and *edit_mode >= edit_mode_type::selectable) {
            // Stop the continues redrawing during dragging.
            // Also reset the time, so on drag-start it will initialize the time, which will
            // cause a smooth startup of repeating.
            _last_drag_mouse_event = {};
            _last_drag_mouse_event_next_repeat = {};
            return true;
        }
        break;

    case mouse_down:
        if (*enabled and *edit_mode >= edit_mode_type::selectable) {
            hilet cursor = _shaped_text.get_nearest_cursor(event.mouse().position);
            switch (event.mouse().click_count) {
            case 1:
                reset_state("BDX");
                _selection = cursor;
                break;
            case 2:
                reset_state("BDX");
                _selection.start_selection(cursor, _shaped_text.select_word(cursor));
                break;
            case 3:
                reset_state("BDX");
                _selection.start_selection(cursor, _shaped_text.select_sentence(cursor));
                break;
            case 4:
                reset_state("BDX");
                _selection.start_selection(cursor, _shaped_text.select_paragraph(cursor));
                break;
            case 5:
                reset_state("BDX");
                _selection.start_selection(cursor, _shaped_text.select_document(cursor));
                break;
            default:;
            }

            request_relayout();
            request_scroll();
            return true;
        }
        break;

    case mouse_drag:
        if (*enabled and *edit_mode >= edit_mode_type::selectable) {
            hilet cursor = _shaped_text.get_nearest_cursor(event.mouse().position);
            switch (event.mouse().click_count) {
            case 1:
                reset_state("BDX");
                _selection.drag_selection(cursor);
                break;
            case 2:
                reset_state("BDX");
                _selection.drag_selection(cursor, _shaped_text.select_word(cursor));
                break;
            case 3:
                reset_state("BDX");
                _selection.drag_selection(cursor, _shaped_text.select_sentence(cursor));
                break;
            case 4:
                reset_state("BDX");
                _selection.drag_selection(cursor, _shaped_text.select_paragraph(cursor));
                break;
            default:;
            }

            // Drag events must be repeated, so that dragging is continues when it causes scrolling.
            // Normally mouse positions are kept in the local coordinate system, but scrolling
            // causes this coordinate system to shift, so translate it to the window coordinate system here.
            _last_drag_mouse_event = event;
            _last_drag_mouse_event.mouse().position = point2{_layout.to_window * event.mouse().position};
            request_redraw();
            return true;
        }
        break;

    default:;
    }

    return super::handle_event(event);
}

hitbox text_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(is_gui_thread());

    if (*visible and *enabled and layout().contains(position)) {
        switch (*edit_mode) {
        case edit_mode_type::selectable:
            return hitbox{this, position, hitbox::Type::Default};
        case edit_mode_type::line_editable:
            return hitbox{this, position, hitbox::Type::TextEdit};
        case edit_mode_type::fully_editable:
            return hitbox{this, position, hitbox::Type::TextEdit};
        default:
            return hitbox{};
        }
    } else {
        return hitbox{};
    }
}

[[nodiscard]] bool text_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    using enum edit_mode_type;
    using enum keyboard_focus_group;

    return *visible and *enabled and any(group & normal) and (*edit_mode == line_editable or *edit_mode == fully_editable);
}

} // namespace hi::inline v1
