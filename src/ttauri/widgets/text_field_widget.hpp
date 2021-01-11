// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "widget.hpp"
#include "../text/EditableText.hpp"
#include "../format.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class text_field_widget final : public widget {
public:
    using super = widget;

    text_field_widget(gui_window &window, std::shared_ptr<widget> parent, std::u8string const label) noexcept :
        widget(window, parent), _label(std::move(label)), _field(theme::global->labelStyle), _shaped_text()
    {
    }

    text_field_widget(gui_window &window, std::shared_ptr<widget> parent, label const label) noexcept :
        text_field_widget(window, parent, label.text())
    {
    }

    ~text_field_widget() {}

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            ttlet maximum_height = _shaped_text.boundingBox.height() + theme::global->margin * 2.0f;

            _preferred_size = {
                f32x4{100.0f, theme::global->smallSize + theme::global->margin * 2.0f},
                f32x4{std::numeric_limits<float>::infinity(), theme::global->smallSize + theme::global->margin * 2.0f}};
            _preferred_base_line = relative_base_line{vertical_alignment::middle, 0.0f, 200.0f};
            _width_resistance = 2;
            return true;
        } else {
            return false;
        }
    }

    void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (_focus && display_time_point >= _next_redraw_time_point) {
            window.request_redraw(window_clipping_rectangle());
        }

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            _text_rectangle = shrink(rectangle(), theme::global->margin);

            // Set the clipping rectangle to within the border of the input field.
            // Add another border width, so glyphs do not touch the border.
            _text_clipping_rectangle =
                intersect(window_clipping_rectangle(), shrink(_window_rectangle, theme::global->borderWidth * 2.0f));

            _field.setStyleOfAll(theme::global->labelStyle);

            if (std::ssize(_field) == 0) {
                _shaped_text =
                    ShapedText(_label, theme::global->placeholderLabelStyle, _text_rectangle.width(), alignment::middle_left);
            } else {
                _field.setWidth(_text_rectangle.width());
                _shaped_text = _field.shapedText();
            }

            // Record the last time the text is modified, so that the caret remains lit.
            _last_update_time_point = display_time_point;
        }

        widget::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        _next_redraw_time_point = display_time_point + _blink_interval;

        if (overlaps(context, this->window_clipping_rectangle())) {
            scroll_text();

            draw_background_box(context);

            // After drawing the border around the input field make sure any other
            // drawing remains inside this border. And change the transform to account
            // for how much the text has scrolled.
            context.clipping_rectangle = _text_clipping_rectangle;
            context.transform = (mat::T(0.0, 0.0, 0.1f) * _text_translate) * context.transform;

            draw_selection_rectangles(context);
            draw_partial_grapheme_caret(context);
            draw_caret(context, display_time_point);
            draw_text(context);
        }

        widget::draw(std::move(context), display_time_point);
    }

    bool handle_command(command command) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = widget::handle_command(command);

        LOG_DEBUG("text_field_widget: Received command: {}", command);
        if (*enabled) {
            switch (command) {
                using enum command;
            case text_edit_paste:
                handled = true;
                _field.handlePaste(window.getTextFromClipboard());
                break;

            case text_edit_copy:
                handled = true;
                window.setTextOnClipboard(_field.handleCopy());
                break;

            case text_edit_cut:
                handled = true;
                window.setTextOnClipboard(_field.handleCut());
                break;

            default: handled |= _field.handle_command(command);
            }
        }

        _request_relayout = true;
        return handled;
    }

    bool handle_mouse_event(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = widget::handle_mouse_event(event);

        // Make sure we only scroll when dragging outside the widget.
        ttlet position = _from_window_transform * event.position;
        _drag_scroll_speed_x = 0.0f;
        _drag_click_count = event.clickCount;
        _drag_select_position = position;

        if (event.cause.leftButton) {
            handled = true;

            if (!*enabled) {
                return true;
            }

            switch (event.type) {
                using enum MouseEvent::Type;
            case ButtonDown:
                if (_text_rectangle.contains(position)) {
                    ttlet mouseInTextPosition = _text_inv_translate * position;

                    switch (event.clickCount) {
                    case 1:
                        if (event.down.shiftKey) {
                            _field.dragCursorAtCoordinate(mouseInTextPosition);
                        } else {
                            _field.setCursorAtCoordinate(mouseInTextPosition);
                        }
                        break;
                    case 2: _field.selectWordAtCoordinate(mouseInTextPosition); break;
                    case 3: _field.selectParagraphAtCoordinate(mouseInTextPosition); break;
                    default:;
                    }

                    // Record the last time the cursor is moved, so that the caret remains lit.
                    _last_update_time_point = event.timePoint;

                    window.request_redraw(window_clipping_rectangle());
                }
                break;

            case Drag:
                // When the mouse is dragged beyond the line input,
                // start scrolling the text and select on the edge of the textRectangle.
                if (position.x() > _text_rectangle.p3().x()) {
                    // The mouse is on the right of the text.
                    _drag_select_position.x() = _text_rectangle.p3().x();

                    // Scroll text to the left in points per second.
                    _drag_scroll_speed_x = 50.0f;

                } else if (position.x() < _text_rectangle.x()) {
                    // The mouse is on the left of the text.
                    _drag_select_position.x() = _text_rectangle.x();

                    // Scroll text to the right in points per second.
                    _drag_scroll_speed_x = -50.0f;
                }

                drag_select();

                window.request_redraw(window_clipping_rectangle());
                break;

            default:;
            }
        }
        return handled;
    }

    bool handle_keyboard_event(KeyboardEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        auto handled = widget::handle_keyboard_event(event);

        if (*enabled) {
            switch (event.type) {
                using enum KeyboardEvent::Type;
            case Grapheme:
                handled = true;
                _field.insertGrapheme(event.grapheme);
                break;

            case PartialGrapheme:
                handled = true;
                _field.insertPartialGrapheme(event.grapheme);
                break;

            default:;
            }
        }

        _request_relayout = true;
        return handled;
    }

    HitBox hitbox_test(f32x4 window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (window_clipping_rectangle().contains(window_position)) {
            return HitBox{weak_from_this(), _draw_layer, *enabled ? HitBox::Type::TextEdit : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool accepts_focus() const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return *enabled;
    }

private:
    std::u8string _label = u8"<unknown>";

    EditableText _field;
    ShapedText _shaped_text;
    aarect _text_rectangle = {};
    aarect _text_clipping_rectangle = {};
    aarect _left_to_right_caret = {};

    /** Scroll speed in points per second.
     * This is used when dragging outside of the widget.
     */
    float _drag_scroll_speed_x = 0.0f;

    /** Number of mouse clicks that caused the drag.
     */
    int _drag_click_count = 0;

    f32x4 _drag_select_position = {};

    /** How much the text has scrolled in points.
     */
    float _text_scroll_x = 0.0f;

    mat::T2 _text_translate;
    mat::T2 _text_inv_translate;

    static constexpr hires_utc_clock::duration _blink_interval = 500ms;
    hires_utc_clock::time_point _next_redraw_time_point;
    hires_utc_clock::time_point _last_update_time_point;

    void drag_select() noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet mouseInTextPosition = _text_inv_translate * _drag_select_position;
        switch (_drag_click_count) {
        case 1: _field.dragCursorAtCoordinate(mouseInTextPosition); break;
        case 2: _field.dragWordAtCoordinate(mouseInTextPosition); break;
        case 3: _field.dragParagraphAtCoordinate(mouseInTextPosition); break;
        default:;
        }
    }

    void scroll_text() noexcept
    {
        if (_drag_scroll_speed_x != 0.0f) {
            _text_scroll_x += _drag_scroll_speed_x * (1.0f / 60.0f);
            drag_select();

            // Once we are scrolling, don't stop.
            window.request_redraw(window_clipping_rectangle());

        } else if (_drag_click_count == 0) {
            // The following is for scrolling based on keyboard input, ignore mouse drags.

            // Scroll the text a quarter width to the left until the cursor is within the width
            // of the text field
            if (_left_to_right_caret.x() - _text_scroll_x > _text_rectangle.width()) {
                _text_scroll_x = _left_to_right_caret.x() - _text_rectangle.width() * 0.75f;
            }

            // Scroll the text a quarter width to the right until the cursor is within the width
            // of the text field
            while (_left_to_right_caret.x() - _text_scroll_x < 0.0f) {
                _text_scroll_x = _left_to_right_caret.x() - _text_rectangle.width() * 0.25f;
            }
        }

        // cap how far we scroll.
        ttlet max_scroll_width = std::max(0.0f, _shaped_text.preferred_extent.width() - _text_rectangle.width());
        _text_scroll_x = std::clamp(_text_scroll_x, 0.0f, max_scroll_width);

        // Calculate how much we need to translate the text.
        _text_translate = mat::T2(-_text_scroll_x, 0.0f) * _shaped_text.T(_text_rectangle);
        _text_inv_translate = ~_text_translate;
    }

    void draw_background_box(draw_context const &context) const noexcept
    {
        context.draw_box_with_border_inside(rectangle());
    }

    void draw_selection_rectangles(draw_context context) const noexcept
    {
        ttlet selection_rectangles = _field.selectionRectangles();
        for (ttlet selection_rectangle : selection_rectangles) {
            context.fill_color = theme::global->textSelectColor;
            context.draw_filled_quad(selection_rectangle);
        }
    }

    void draw_partial_grapheme_caret(draw_context context) const noexcept
    {
        ttlet partial_grapheme_caret = _field.partialGraphemeCaret();
        if (partial_grapheme_caret) {
            context.fill_color = theme::global->incompleteGlyphColor;
            context.draw_filled_quad(partial_grapheme_caret);
        }
    }

    void draw_caret(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
    {
        // Display the caret and handle blinking.
        ttlet duration_since_last_update = display_time_point - _last_update_time_point;
        ttlet nr_half_blinks = static_cast<int64_t>(duration_since_last_update / _blink_interval);

        ttlet blink_is_on = nr_half_blinks % 2 == 0;
        _left_to_right_caret = _field.leftToRightCaret();
        if (_left_to_right_caret && blink_is_on && _focus && window.active) {
            context.fill_color = theme::global->cursorColor;
            context.draw_filled_quad(_left_to_right_caret);
        }
    }

    void draw_text(draw_context context) const noexcept
    {
        context.transform = mat::T(0.0f, 0.0f, 0.2f) * context.transform;
        context.draw_text(_shaped_text);
    }
};

} // namespace tt
