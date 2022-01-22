// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"

namespace tt::inline v1{

    text_widget::text_widget(gui_window & window, widget * parent) noexcept : super(window, parent)
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

        if (enabled and visible and edit_mode == edit_mode_type::editable) {
            context.draw_text_cursors(layout(), _shaped_text, _selection.cursor(), theme().color(theme_color::cursor), theme().color(theme_color::incomplete_glyph));
        }
    }
}

[[nodiscard]] gstring_view text_widget::selected_text() const noexcept
{
    ttlet[first, last] = _selection.selection_indices();
    ttlet &text_ = *text.cget();
    tt_axiom(first <= last and last <= text_.size());

    return gstring_view{text_}.substr(first, last - first);
}

bool text_widget::handle_event(tt::command command) noexcept
{
    tt_axiom(is_gui_thread());
    request_relayout();

    if (enabled) {
        switch (command) {
            //case command::text_edit_paste:
            //    _field.handle_paste(window.get_text_from_clipboard());
            //    commit(false);
            //    return true;
            //
        case command::text_edit_copy:
            if (ttlet selected_text_ = selected_text(); not selected_text_.empty()) {
                window.set_text_on_clipboard(to_string(selected_text_));
            }
            return true;

            //
            //case command::text_edit_cut: window.set_text_on_clipboard(_field.handle_cut()); return true;

        case command::gui_cancel:
            _selection.clear_selection();
            return true;

        default:;
        }
    }

    return super::handle_event(command);
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

        ttlet index = _shaped_text.get_nearest(event.position);

        switch (event.type) {
            using enum mouse_event::Type;
        case ButtonDown:
            switch (event.clickCount) {
            case 1:
                _selection.set_cursor(index);
                break;
            case 2:
                //_selection.start_selection(index, _shaped_text.get_word(index));
                break;
            case 3:
                //_selection.start_selection(index, _shaped_text.get_sentence(index));
                break;
            default:;
            }

            // Record the last time the cursor is moved, so that the caret remains lit.
            //_last_update_time_point = event.timePoint;

            request_redraw();
            break;

        case Drag:
            switch (event.clickCount) {
            case 1:
                _selection.drag_selection(index);
                break;
            case 2:
                //_selection.drag_selection(index, _shaped_text.get_word(index));
                break;
            case 3:
                //_selection.drag_selection(index, _shaped_text.get_sentence(index));
                break;
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
    return visible and enabled and edit_mode != edit_mode_type::fixed and any(group & tt::keyboard_focus_group::normal);
}


} // namespace tt::inline v1
