// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"
#include "../GUI/mouse_event.hpp"

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

        context.draw_text_cursors(layout(), _shaped_text, _selection.cursor(), theme().color(theme_color::cursor), theme().color(theme_color::incomplete_glyph));
    }
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
        return hitbox{this, position, hitbox::Type::TextEdit};
    } else {
        return hitbox{};
    }
}

} // namespace tt::inline v1
