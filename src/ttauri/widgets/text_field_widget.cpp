// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_field_widget.hpp"
#include "../text/font_book.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/gui_system.hpp"

namespace tt::inline v1 {

text_field_widget::text_field_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept :
    super(window, parent),
    _delegate(std::move(delegate)),
    _field(font_book(), theme().text_style(theme_text_style::label)),
    _shaped_text()
{
    if (auto d = _delegate.lock()) {
        d->subscribe(*this, _relayout_callback);
        d->init(*this);
    }
    continues.subscribe(_relayout_callback);
    text_style.subscribe(_relayout_callback);
    text_width.subscribe(_reconstrain_callback);
    _error_label_widget =
        std::make_unique<label_widget>(window, this, _error_label, alignment::top_left(), theme_text_style::error);
}

text_field_widget::text_field_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
    text_field_widget(window, parent, weak_or_unique_ptr<delegate_type>{std::move(delegate)})
{
}

text_field_widget::~text_field_widget()
{
    if (auto delegate = _delegate.lock()) {
        delegate->deinit(*this);
    }
}

widget_constraints const &text_field_widget::set_constraints() noexcept
{
    _layout = {};

    ttlet error_label_constraints = _error_label_widget->set_constraints();
    ttlet error_size = _error_label ? error_label_constraints.preferred + extent2{0.0f, theme().margin} : extent2{};

    auto size = extent2{
        std::max(text_width + theme().margin * 2.0f, error_size.width()),
        theme().size + theme().margin * 2.0f + error_size.height()};
    return _constraints = {size, size, size, theme().margin};
}

void text_field_widget::set_layout(widget_layout const &layout) noexcept
{
    ttlet text_was_modified = std::exchange(_text_was_modified, false);
    if (compare_store(_layout, layout) or text_was_modified) {
        ttlet text_field_height = theme().size + theme().margin * 2.0f;
        ttlet error_label_rectangle = aarectangle{
            extent2{layout.width(), std::min(layout.height(), _error_label_widget->constraints().preferred.height())}};
        _error_label_widget->visible = not _error_label->empty();
        _error_label_widget->set_layout(layout.transform(error_label_rectangle));

        // The rectangle is a single line, but at full width. Aligned to the top of the widget.
        ttlet text_field_size = extent2{layout.width(), text_field_height};
        _text_field_rectangle = aarectangle{point2{0.0f, layout.height() - text_field_height}, text_field_size};

        // Set the clipping rectangle to within the border of the input field.
        // Add another border width, so glyphs do not touch the border.
        _text_field_clipping_rectangle = intersect(layout.clipping_rectangle, _text_field_rectangle);

        _text_rectangle = _text_field_rectangle - theme().margin;

        ttlet field_str = static_cast<std::string>(_field);

        if (focus) {
            // Update the optional error value from the string conversion when the
            // field has keyboard focus.
            if (auto delegate = _delegate.lock()) {
                _error_label = delegate->validate(*this, field_str);
            } else {
                _error_label = {};
            }

        } else {
            // When field is not focused, simply follow the observed_value.
            if (auto delegate = _delegate.lock()) {
                _field = delegate->text(*this);
            } else {
                _field = {};
            }
            _error_label = {};
        }

        _field.set_style_of_all(theme().text_style(theme_text_style::label));
        _field.set_width(std::numeric_limits<float>::infinity());
        _shaped_text = _field.shaped_text();

        // Record the last time the text is modified, so that the caret remains lit.
        _last_update_time_point = layout.display_time_point;
    }
}

void text_field_widget::draw(draw_context const &context) noexcept
{
    _next_redraw_time_point = context.display_time_point + _blink_interval;

    if (visible and overlaps(context, layout())) {
        scroll_text();

        draw_background_box(context);

        // After drawing the border around the input field make sure any other
        // drawing remains inside this border. And change the transform to account
        // for how much the text has scrolled.
        draw_selection_rectangles(context);
        draw_partial_grapheme_caret(context);
        draw_caret(context);
        draw_text(context);
    }

    _error_label_widget->draw(context);
}

bool text_field_widget::handle_event(command command) noexcept
{
    tt_axiom(is_gui_thread());
    _text_was_modified = true;
    request_relayout();

    if (enabled) {
        switch (command) {
        case command::text_edit_paste:
            _field.handle_paste(window.get_text_from_clipboard());
            commit(false);
            return true;

        case command::text_edit_copy: window.set_text_on_clipboard(_field.handle_copy()); return true;

        case command::text_edit_cut: window.set_text_on_clipboard(_field.handle_cut()); return true;

        case command::gui_cancel: revert(true); return true;

        case command::gui_enter:
            commit(true);
            this->window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::forward);
            return true;

        case command::gui_keyboard_enter:
            revert(true);
            // More processing of the gui_keyboard_enter command is required.
            break;

        case command::gui_keyboard_exit:
            commit(true);
            // More processing of the gui_keyboard_exit command is required.
            break;

        default:
            if (_field.handle_event(command)) {
                commit(false);
                return true;
            }
        }
    }

    return super::handle_event(command);
}

bool text_field_widget::handle_event(mouse_event const &event) noexcept
{
    tt_axiom(is_gui_thread());
    auto handled = super::handle_event(event);

    // Make sure we only scroll when dragging outside the widget.
    _drag_scroll_speed_x = 0.0f;
    _drag_click_count = event.clickCount;
    _drag_select_position = event.position;

    if (event.cause.leftButton) {
        handled = true;

        if (not *enabled) {
            return true;
        }

        switch (event.type) {
            using enum mouse_event::Type;
        case ButtonDown:
            if (_text_rectangle.contains(event.position)) {
                ttlet mouse_cursor_relative_to_text = _text_inv_translate * event.position;

                switch (event.clickCount) {
                case 1:
                    if (event.down.shiftKey) {
                        _field.drag_cursor_at_coordinate(mouse_cursor_relative_to_text);
                    } else {
                        _field.set_cursor_at_coordinate(mouse_cursor_relative_to_text);
                    }
                    break;
                case 2: _field.select_word_at_coordinate(mouse_cursor_relative_to_text); break;
                case 3: _field.select_paragraph_at_coordinate(mouse_cursor_relative_to_text); break;
                default:;
                }

                // Record the last time the cursor is moved, so that the caret remains lit.
                _last_update_time_point = event.timePoint;

                request_redraw();
            }
            break;

        case Drag:
            // When the mouse is dragged beyond the line input,
            // start scrolling the text and select on the edge of the textRectangle.
            if (event.position.x() > _text_rectangle.right()) {
                // The mouse is on the right of the text.
                _drag_select_position.x() = _text_rectangle.right();

                // Scroll text to the left in points per second.
                _drag_scroll_speed_x = 50.0f;

            } else if (event.position.x() < _text_rectangle.left()) {
                // The mouse is on the left of the text.
                _drag_select_position.x() = _text_rectangle.left();

                // Scroll text to the right in points per second.
                _drag_scroll_speed_x = -50.0f;
            }

            drag_select();

            request_redraw();
            break;

        default:;
        }
    }
    return handled;
}

bool text_field_widget::handle_event(keyboard_event const &event) noexcept
{
    tt_axiom(is_gui_thread());

    auto handled = super::handle_event(event);

    if (enabled) {
        switch (event.type) {
            using enum keyboard_event::Type;

        case grapheme:
            handled = true;
            _field.insert_grapheme(event.grapheme);
            commit(false);
            break;

        case Partialgrapheme:
            handled = true;
            _field.insert_partial_grapheme(event.grapheme);
            commit(false);
            break;

        default:;
        }
    }

    _text_was_modified = true;
    request_relayout();
    return handled;
}

hitbox text_field_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and enabled and layout().contains(position) and _text_field_rectangle.contains(position)) {
        return hitbox{this, position, hitbox::Type::TextEdit};
    } else {
        return hitbox{};
    }
}

[[nodiscard]] bool text_field_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    return visible and enabled and any(group & tt::keyboard_focus_group::normal);
}

[[nodiscard]] color text_field_widget::focus_color() const noexcept
{
    if (enabled and active() and not _error_label->empty()) {
        return theme().text_style(theme_text_style::error).color;
    } else {
        return super::focus_color();
    }
}

void text_field_widget::revert(bool force) noexcept
{
    if (auto delegate = _delegate.lock()) {
        _field = delegate->text(*this);
    } else {
        _field = {};
    }
    _error_label = {};
}

void text_field_widget::commit(bool force) noexcept
{
    tt_axiom(is_gui_thread());
    if (_continues || force) {
        auto text = static_cast<std::string>(_field);

        if (auto delegate = _delegate.lock()) {
            if (delegate->validate(*this, text).empty()) {
                // text is valid.
                delegate->set_text(*this, text);
            }

            // After commit get the canonical text to display from the delegate.
            _field = delegate->text(*this);
        } else {
            _field = {};
        }
        _error_label = {};
    }
}

void text_field_widget::drag_select() noexcept
{
    tt_axiom(is_gui_thread());

    ttlet mouse_cursor_relative_to_text = _text_inv_translate * _drag_select_position;
    switch (_drag_click_count) {
    case 1: _field.drag_cursor_at_coordinate(mouse_cursor_relative_to_text); break;
    case 2: _field.drag_word_at_coordinate(mouse_cursor_relative_to_text); break;
    case 3: _field.drag_paragraph_at_coordinate(mouse_cursor_relative_to_text); break;
    default:;
    }
}

void text_field_widget::scroll_text() noexcept
{
    if (_drag_scroll_speed_x != 0.0f) {
        _text_scroll_x += _drag_scroll_speed_x * (1.0f / 60.0f);
        drag_select();

        // Once we are scrolling, don't stop.
        request_redraw();

    } else if (_drag_click_count == 0) {
        // The following is for scrolling based on keyboard input, ignore mouse drags.

        // Scroll the text a quarter width to the left until the cursor is within the width
        // of the text field
        if (_left_to_right_caret.left() - _text_scroll_x > _text_rectangle.width()) {
            _text_scroll_x = _left_to_right_caret.left() - _text_rectangle.width() * 0.75f;
        }

        // Scroll the text a quarter width to the right until the cursor is within the width
        // of the text field
        while (_left_to_right_caret.left() - _text_scroll_x < 0.0f) {
            _text_scroll_x = _left_to_right_caret.left() - _text_rectangle.width() * 0.25f;
        }
    }

    // cap how far we scroll.
    ttlet max_scroll_width = std::max(0.0f, _shaped_text.preferred_size().width() - _text_rectangle.width());
    _text_scroll_x = std::clamp(_text_scroll_x, 0.0f, max_scroll_width);

    // Calculate how much we need to translate the text.
    _text_translate = translate2{-_text_scroll_x, 0.0f} *
        _shaped_text.translate_base_line(point2{_text_rectangle.left(), _text_rectangle.middle()});
    _text_inv_translate = ~_text_translate;
}

void text_field_widget::draw_background_box(draw_context const &context) const noexcept
{
    ttlet corner_radii = tt::corner_radii{0.0f, 0.0f, theme().rounding_radius, theme().rounding_radius};
    context.draw_box(_layout, _text_field_rectangle, background_color(), corner_radii);

    ttlet line = line_segment(get<0>(_text_field_rectangle), get<1>(_text_field_rectangle));
    context.draw_line(_layout, translate3{0.0f, 0.5f, 0.1f} * line, theme().border_width, focus_color());
}

void text_field_widget::draw_selection_rectangles(draw_context const &context) const noexcept
{
    ttlet selection_rectangles = _field.selection_rectangles();
    for (ttlet selection_rectangle : selection_rectangles) {
        context.draw_box(
            _layout, _text_translate * translate_z(0.1f) * selection_rectangle, theme().color(theme_color::text_select));
    }
}

void text_field_widget::draw_partial_grapheme_caret(draw_context const &context)
    const noexcept
{
    ttlet partial_grapheme_caret = _field.partial_grapheme_caret();
    if (partial_grapheme_caret) {
        ttlet box = round(_text_translate) * translate_z(0.1f) * round(partial_grapheme_caret);
        context.draw_box(
            _layout, box, color::transparent(), theme().color(theme_color::incomplete_glyph), 1.0f, border_side::outside);
    }
}

void text_field_widget::draw_caret(draw_context const &context) noexcept
{
    if (focus and active()) {
        // Keep redrawing while the text-field has focus.
        request_redraw();

        // Display the caret and handle blinking.
        ttlet duration_since_last_update = context.display_time_point - _last_update_time_point;
        ttlet nr_half_blinks = static_cast<int64_t>(duration_since_last_update / _blink_interval);
        ttlet blink_is_on = nr_half_blinks % 2 == 0;

        _left_to_right_caret = _field.left_to_right_caret();
        if (_left_to_right_caret and blink_is_on) {
            ttlet box = round(_text_translate) * translate_z(0.1f) * round(_left_to_right_caret);
            context.draw_box(
                _layout,
                _text_field_clipping_rectangle,
                box,
                color::transparent(),
                theme().color(theme_color::cursor),
                1.0f,
                border_side::inside);
        }

        //ttlet right_to_left_caret = _field.right_to_left_caret();
        //if (right_to_left_caret and blink_is_on) {
        //    ttlet box = round(_text_translate) * translate_z(0.1f) * round(right_to_left_caret);
        //    context.draw_box(
        //        clipped_layout, box, color::transparent(), theme().color(theme_color::red), 1.0f, //border_side::inside);
        //}

    }
}

void text_field_widget::draw_text(draw_context const &context) const noexcept
{
    context.draw_text(_layout, _text_field_clipping_rectangle, _text_translate * translate_z(0.2f), label_color(), _shaped_text);
}

} // namespace tt::inline v1
